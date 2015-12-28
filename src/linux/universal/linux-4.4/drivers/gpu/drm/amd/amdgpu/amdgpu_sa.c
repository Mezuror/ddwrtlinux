/*
 * Copyright 2011 Red Hat Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 */
/*
 * Authors:
 *    Jerome Glisse <glisse@freedesktop.org>
 */
/* Algorithm:
 *
 * We store the last allocated bo in "hole", we always try to allocate
 * after the last allocated bo. Principle is that in a linear GPU ring
 * progression was is after last is the oldest bo we allocated and thus
 * the first one that should no longer be in use by the GPU.
 *
 * If it's not the case we skip over the bo after last to the closest
 * done bo if such one exist. If none exist and we are not asked to
 * block we report failure to allocate.
 *
 * If we are asked to block we wait on all the oldest fence of all
 * rings. We just wait for any of those fence to complete.
 */
#include <drm/drmP.h>
#include "amdgpu.h"

static void amdgpu_sa_bo_remove_locked(struct amdgpu_sa_bo *sa_bo);
static void amdgpu_sa_bo_try_free(struct amdgpu_sa_manager *sa_manager);

int amdgpu_sa_bo_manager_init(struct amdgpu_device *adev,
			      struct amdgpu_sa_manager *sa_manager,
			      unsigned size, u32 align, u32 domain)
{
	int i, r;

	init_waitqueue_head(&sa_manager->wq);
	sa_manager->bo = NULL;
	sa_manager->size = size;
	sa_manager->domain = domain;
	sa_manager->align = align;
	sa_manager->hole = &sa_manager->olist;
	INIT_LIST_HEAD(&sa_manager->olist);
	for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
		INIT_LIST_HEAD(&sa_manager->flist[i]);
	}

	r = amdgpu_bo_create(adev, size, align, true, domain,
			     0, NULL, NULL, &sa_manager->bo);
	if (r) {
		dev_err(adev->dev, "(%d) failed to allocate bo for manager\n", r);
		return r;
	}

	return r;
}

void amdgpu_sa_bo_manager_fini(struct amdgpu_device *adev,
			       struct amdgpu_sa_manager *sa_manager)
{
	struct amdgpu_sa_bo *sa_bo, *tmp;

	if (!list_empty(&sa_manager->olist)) {
		sa_manager->hole = &sa_manager->olist,
		amdgpu_sa_bo_try_free(sa_manager);
		if (!list_empty(&sa_manager->olist)) {
			dev_err(adev->dev, "sa_manager is not empty, clearing anyway\n");
		}
	}
	list_for_each_entry_safe(sa_bo, tmp, &sa_manager->olist, olist) {
		amdgpu_sa_bo_remove_locked(sa_bo);
	}
	amdgpu_bo_unref(&sa_manager->bo);
	sa_manager->size = 0;
}

int amdgpu_sa_bo_manager_start(struct amdgpu_device *adev,
			       struct amdgpu_sa_manager *sa_manager)
{
	int r;

	if (sa_manager->bo == NULL) {
		dev_err(adev->dev, "no bo for sa manager\n");
		return -EINVAL;
	}

	/* map the buffer */
	r = amdgpu_bo_reserve(sa_manager->bo, false);
	if (r) {
		dev_err(adev->dev, "(%d) failed to reserve manager bo\n", r);
		return r;
	}
	r = amdgpu_bo_pin(sa_manager->bo, sa_manager->domain, &sa_manager->gpu_addr);
	if (r) {
		amdgpu_bo_unreserve(sa_manager->bo);
		dev_err(adev->dev, "(%d) failed to pin manager bo\n", r);
		return r;
	}
	r = amdgpu_bo_kmap(sa_manager->bo, &sa_manager->cpu_ptr);
	amdgpu_bo_unreserve(sa_manager->bo);
	return r;
}

int amdgpu_sa_bo_manager_suspend(struct amdgpu_device *adev,
				 struct amdgpu_sa_manager *sa_manager)
{
	int r;

	if (sa_manager->bo == NULL) {
		dev_err(adev->dev, "no bo for sa manager\n");
		return -EINVAL;
	}

	r = amdgpu_bo_reserve(sa_manager->bo, false);
	if (!r) {
		amdgpu_bo_kunmap(sa_manager->bo);
		amdgpu_bo_unpin(sa_manager->bo);
		amdgpu_bo_unreserve(sa_manager->bo);
	}
	return r;
}

static void amdgpu_sa_bo_remove_locked(struct amdgpu_sa_bo *sa_bo)
{
	struct amdgpu_sa_manager *sa_manager = sa_bo->manager;
	if (sa_manager->hole == &sa_bo->olist) {
		sa_manager->hole = sa_bo->olist.prev;
	}
	list_del_init(&sa_bo->olist);
	list_del_init(&sa_bo->flist);
	fence_put(sa_bo->fence);
	kfree(sa_bo);
}

static void amdgpu_sa_bo_try_free(struct amdgpu_sa_manager *sa_manager)
{
	struct amdgpu_sa_bo *sa_bo, *tmp;

	if (sa_manager->hole->next == &sa_manager->olist)
		return;

	sa_bo = list_entry(sa_manager->hole->next, struct amdgpu_sa_bo, olist);
	list_for_each_entry_safe_from(sa_bo, tmp, &sa_manager->olist, olist) {
		if (sa_bo->fence == NULL ||
		    !fence_is_signaled(sa_bo->fence)) {
			return;
		}
		amdgpu_sa_bo_remove_locked(sa_bo);
	}
}

static inline unsigned amdgpu_sa_bo_hole_soffset(struct amdgpu_sa_manager *sa_manager)
{
	struct list_head *hole = sa_manager->hole;

	if (hole != &sa_manager->olist) {
		return list_entry(hole, struct amdgpu_sa_bo, olist)->eoffset;
	}
	return 0;
}

static inline unsigned amdgpu_sa_bo_hole_eoffset(struct amdgpu_sa_manager *sa_manager)
{
	struct list_head *hole = sa_manager->hole;

	if (hole->next != &sa_manager->olist) {
		return list_entry(hole->next, struct amdgpu_sa_bo, olist)->soffset;
	}
	return sa_manager->size;
}

static bool amdgpu_sa_bo_try_alloc(struct amdgpu_sa_manager *sa_manager,
				   struct amdgpu_sa_bo *sa_bo,
				   unsigned size, unsigned align)
{
	unsigned soffset, eoffset, wasted;

	soffset = amdgpu_sa_bo_hole_soffset(sa_manager);
	eoffset = amdgpu_sa_bo_hole_eoffset(sa_manager);
	wasted = (align - (soffset % align)) % align;

	if ((eoffset - soffset) >= (size + wasted)) {
		soffset += wasted;

		sa_bo->manager = sa_manager;
		sa_bo->soffset = soffset;
		sa_bo->eoffset = soffset + size;
		list_add(&sa_bo->olist, sa_manager->hole);
		INIT_LIST_HEAD(&sa_bo->flist);
		sa_manager->hole = &sa_bo->olist;
		return true;
	}
	return false;
}

/**
 * amdgpu_sa_event - Check if we can stop waiting
 *
 * @sa_manager: pointer to the sa_manager
 * @size: number of bytes we want to allocate
 * @align: alignment we need to match
 *
 * Check if either there is a fence we can wait for or
 * enough free memory to satisfy the allocation directly
 */
static bool amdgpu_sa_event(struct amdgpu_sa_manager *sa_manager,
			    unsigned size, unsigned align)
{
	unsigned soffset, eoffset, wasted;
	int i;

	for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
		if (!list_empty(&sa_manager->flist[i])) {
			return true;
		}
	}

	soffset = amdgpu_sa_bo_hole_soffset(sa_manager);
	eoffset = amdgpu_sa_bo_hole_eoffset(sa_manager);
	wasted = (align - (soffset % align)) % align;

	if ((eoffset - soffset) >= (size + wasted)) {
		return true;
	}

	return false;
}

static bool amdgpu_sa_bo_next_hole(struct amdgpu_sa_manager *sa_manager,
				   struct fence **fences,
				   unsigned *tries)
{
	struct amdgpu_sa_bo *best_bo = NULL;
	unsigned i, soffset, best, tmp;

	/* if hole points to the end of the buffer */
	if (sa_manager->hole->next == &sa_manager->olist) {
		/* try again with its beginning */
		sa_manager->hole = &sa_manager->olist;
		return true;
	}

	soffset = amdgpu_sa_bo_hole_soffset(sa_manager);
	/* to handle wrap around we add sa_manager->size */
	best = sa_manager->size * 2;
	/* go over all fence list and try to find the closest sa_bo
	 * of the current last
	 */
	for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
		struct amdgpu_sa_bo *sa_bo;

		if (list_empty(&sa_manager->flist[i])) {
			continue;
		}

		sa_bo = list_first_entry(&sa_manager->flist[i],
					 struct amdgpu_sa_bo, flist);

		if (!fence_is_signaled(sa_bo->fence)) {
			fences[i] = sa_bo->fence;
			continue;
		}

		/* limit the number of tries each ring gets */
		if (tries[i] > 2) {
			continue;
		}

		tmp = sa_bo->soffset;
		if (tmp < soffset) {
			/* wrap around, pretend it's after */
			tmp += sa_manager->size;
		}
		tmp -= soffset;
		if (tmp < best) {
			/* this sa bo is the closest one */
			best = tmp;
			best_bo = sa_bo;
		}
	}

	if (best_bo) {
		uint32_t idx = amdgpu_ring_from_fence(best_bo->fence)->idx;
		++tries[idx];
		sa_manager->hole = best_bo->olist.prev;

		/* we knew that this one is signaled,
		   so it's save to remote it */
		amdgpu_sa_bo_remove_locked(best_bo);
		return true;
	}
	return false;
}

int amdgpu_sa_bo_new(struct amdgpu_sa_manager *sa_manager,
		     struct amdgpu_sa_bo **sa_bo,
		     unsigned size, unsigned align)
{
	struct fence *fences[AMDGPU_MAX_RINGS];
	unsigned tries[AMDGPU_MAX_RINGS];
	unsigned count;
	int i, r;
	signed long t;

	BUG_ON(align > sa_manager->align);
	BUG_ON(size > sa_manager->size);

	*sa_bo = kmalloc(sizeof(struct amdgpu_sa_bo), GFP_KERNEL);
	if ((*sa_bo) == NULL) {
		return -ENOMEM;
	}
	(*sa_bo)->manager = sa_manager;
	(*sa_bo)->fence = NULL;
	INIT_LIST_HEAD(&(*sa_bo)->olist);
	INIT_LIST_HEAD(&(*sa_bo)->flist);

	spin_lock(&sa_manager->wq.lock);
	do {
		for (i = 0; i < AMDGPU_MAX_RINGS; ++i) {
			fences[i] = NULL;
			tries[i] = 0;
		}

		do {
			amdgpu_sa_bo_try_free(sa_manager);

			if (amdgpu_sa_bo_try_alloc(sa_manager, *sa_bo,
						   size, align)) {
				spin_unlock(&sa_manager->wq.lock);
				return 0;
			}

			/* see if we can skip over some allocations */
		} while (amdgpu_sa_bo_next_hole(sa_manager, fences, tries));

		for (i = 0, count = 0; i < AMDGPU_MAX_RINGS; ++i)
			if (fences[i])
				fences[count++] = fences[i];

		if (count) {
			spin_unlock(&sa_manager->wq.lock);
			t = fence_wait_any_timeout(fences, count, false,
						   MAX_SCHEDULE_TIMEOUT);
			r = (t > 0) ? 0 : t;
			spin_lock(&sa_manager->wq.lock);
		} else {
			/* if we have nothing to wait for block */
			r = wait_event_interruptible_locked(
				sa_manager->wq,
				amdgpu_sa_event(sa_manager, size, align)
			);
		}

	} while (!r);

	spin_unlock(&sa_manager->wq.lock);
	kfree(*sa_bo);
	*sa_bo = NULL;
	return r;
}

void amdgpu_sa_bo_free(struct amdgpu_device *adev, struct amdgpu_sa_bo **sa_bo,
		       struct fence *fence)
{
	struct amdgpu_sa_manager *sa_manager;

	if (sa_bo == NULL || *sa_bo == NULL) {
		return;
	}

	sa_manager = (*sa_bo)->manager;
	spin_lock(&sa_manager->wq.lock);
	if (fence && !fence_is_signaled(fence)) {
		uint32_t idx;
		(*sa_bo)->fence = fence_get(fence);
		idx = amdgpu_ring_from_fence(fence)->idx;
		list_add_tail(&(*sa_bo)->flist, &sa_manager->flist[idx]);
	} else {
		amdgpu_sa_bo_remove_locked(*sa_bo);
	}
	wake_up_all_locked(&sa_manager->wq);
	spin_unlock(&sa_manager->wq.lock);
	*sa_bo = NULL;
}

#if defined(CONFIG_DEBUG_FS)

static void amdgpu_sa_bo_dump_fence(struct fence *fence, struct seq_file *m)
{
	struct amdgpu_fence *a_fence = to_amdgpu_fence(fence);
	struct amd_sched_fence *s_fence = to_amd_sched_fence(fence);

	if (a_fence)
		seq_printf(m, " protected by 0x%016llx on ring %d",
			   a_fence->seq, a_fence->ring->idx);

	if (s_fence) {
		struct amdgpu_ring *ring;


		ring = container_of(s_fence->sched, struct amdgpu_ring, sched);
		seq_printf(m, " protected by 0x%016x on ring %d",
			   s_fence->base.seqno, ring->idx);
	}
}

void amdgpu_sa_bo_dump_debug_info(struct amdgpu_sa_manager *sa_manager,
				  struct seq_file *m)
{
	struct amdgpu_sa_bo *i;

	spin_lock(&sa_manager->wq.lock);
	list_for_each_entry(i, &sa_manager->olist, olist) {
		uint64_t soffset = i->soffset + sa_manager->gpu_addr;
		uint64_t eoffset = i->eoffset + sa_manager->gpu_addr;
		if (&i->olist == sa_manager->hole) {
			seq_printf(m, ">");
		} else {
			seq_printf(m, " ");
		}
		seq_printf(m, "[0x%010llx 0x%010llx] size %8lld",
			   soffset, eoffset, eoffset - soffset);
		if (i->fence)
			amdgpu_sa_bo_dump_fence(i->fence, m);
		seq_printf(m, "\n");
	}
	spin_unlock(&sa_manager->wq.lock);
}
#endif
