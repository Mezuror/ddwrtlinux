/* Authentication token and access key management
 *
 * Copyright (C) 2004, 2007 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 * See Documentation/security/keys.txt for information on keys/keyrings.
 */

#ifndef _LINUX_KEY_H
#define _LINUX_KEY_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/rcupdate.h>
#include <linux/sysctl.h>
#include <linux/rwsem.h>
#include <linux/atomic.h>
#include <linux/assoc_array.h>

#ifdef __KERNEL__
#include <linux/uidgid.h>

/* key handle serial number */
typedef int32_t key_serial_t;

/* key handle permissions mask */
typedef uint32_t key_perm_t;

struct key;

#ifdef CONFIG_KEYS

#undef KEY_DEBUGGING

#define KEY_POS_VIEW	0x01000000	/* possessor can view a key's attributes */
#define KEY_POS_READ	0x02000000	/* possessor can read key payload / view keyring */
#define KEY_POS_WRITE	0x04000000	/* possessor can update key payload / add link to keyring */
#define KEY_POS_SEARCH	0x08000000	/* possessor can find a key in search / search a keyring */
#define KEY_POS_LINK	0x10000000	/* possessor can create a link to a key/keyring */
#define KEY_POS_SETATTR	0x20000000	/* possessor can set key attributes */
#define KEY_POS_ALL	0x3f000000

#define KEY_USR_VIEW	0x00010000	/* user permissions... */
#define KEY_USR_READ	0x00020000
#define KEY_USR_WRITE	0x00040000
#define KEY_USR_SEARCH	0x00080000
#define KEY_USR_LINK	0x00100000
#define KEY_USR_SETATTR	0x00200000
#define KEY_USR_ALL	0x003f0000

#define KEY_GRP_VIEW	0x00000100	/* group permissions... */
#define KEY_GRP_READ	0x00000200
#define KEY_GRP_WRITE	0x00000400
#define KEY_GRP_SEARCH	0x00000800
#define KEY_GRP_LINK	0x00001000
#define KEY_GRP_SETATTR	0x00002000
#define KEY_GRP_ALL	0x00003f00

#define KEY_OTH_VIEW	0x00000001	/* third party permissions... */
#define KEY_OTH_READ	0x00000002
#define KEY_OTH_WRITE	0x00000004
#define KEY_OTH_SEARCH	0x00000008
#define KEY_OTH_LINK	0x00000010
#define KEY_OTH_SETATTR	0x00000020
#define KEY_OTH_ALL	0x0000003f

#define KEY_PERM_UNDEF	0xffffffff

struct seq_file;
struct user_struct;
struct signal_struct;
struct cred;

struct key_type;
struct key_owner;
struct keyring_list;
struct keyring_name;

struct keyring_index_key {
	struct key_type		*type;
	const char		*description;
	size_t			desc_len;
};

union key_payload {
	void __rcu		*rcu_data0;
	void			*data[4];
};

/*****************************************************************************/
/*
 * key reference with possession attribute handling
 *
 * NOTE! key_ref_t is a typedef'd pointer to a type that is not actually
 * defined. This is because we abuse the bottom bit of the reference to carry a
 * flag to indicate whether the calling process possesses that key in one of
 * its keyrings.
 *
 * the key_ref_t has been made a separate type so that the compiler can reject
 * attempts to dereference it without proper conversion.
 *
 * the three functions are used to assemble and disassemble references
 */
typedef struct __key_reference_with_attributes *key_ref_t;

static inline key_ref_t make_key_ref(const struct key *key,
				     bool possession)
{
	return (key_ref_t) ((unsigned long) key | possession);
}

static inline struct key *key_ref_to_ptr(const key_ref_t key_ref)
{
	return (struct key *) ((unsigned long) key_ref & ~1UL);
}

static inline bool is_key_possessed(const key_ref_t key_ref)
{
	return (unsigned long) key_ref & 1UL;
}

enum key_state {
	KEY_IS_UNINSTANTIATED,
	KEY_IS_POSITIVE,		/* Positively instantiated */
};

/*****************************************************************************/
/*
 * authentication token / access credential / keyring
 * - types of key include:
 *   - keyrings
 *   - disk encryption IDs
 *   - Kerberos TGTs and tickets
 */
struct key {
	atomic_t		usage;		/* number of references */
	key_serial_t		serial;		/* key serial number */
	union {
		struct list_head graveyard_link;
		struct rb_node	serial_node;
	};
	struct rw_semaphore	sem;		/* change vs change sem */
	struct key_user		*user;		/* owner of this key */
	void			*security;	/* security data for this key */
	union {
		time_t		expiry;		/* time at which key expires (or 0) */
		time_t		revoked_at;	/* time at which key was revoked */
	};
	time_t			last_used_at;	/* last time used for LRU keyring discard */
	kuid_t			uid;
	kgid_t			gid;
	key_perm_t		perm;		/* access permissions */
	unsigned short		quotalen;	/* length added to quota */
	unsigned short		datalen;	/* payload data length
						 * - may not match RCU dereferenced payload
						 * - payload should contain own length
						 */
	short			state;		/* Key state (+) or rejection error (-) */

#ifdef KEY_DEBUGGING
	unsigned		magic;
#define KEY_DEBUG_MAGIC		0x18273645u
#define KEY_DEBUG_MAGIC_X	0xf8e9dacbu
#endif

	unsigned long		flags;		/* status flags (change with bitops) */
#define KEY_FLAG_DEAD		0	/* set if key type has been deleted */
#define KEY_FLAG_REVOKED	1	/* set if key had been revoked */
#define KEY_FLAG_IN_QUOTA	2	/* set if key consumes quota */
#define KEY_FLAG_USER_CONSTRUCT	3	/* set if key is being constructed in userspace */
#define KEY_FLAG_ROOT_CAN_CLEAR	4	/* set if key can be cleared by root without permission */
#define KEY_FLAG_INVALIDATED	5	/* set if key has been invalidated */
#define KEY_FLAG_TRUSTED	6	/* set if key is trusted */
#define KEY_FLAG_TRUSTED_ONLY	7	/* set if keyring only accepts links to trusted keys */
#define KEY_FLAG_BUILTIN	8	/* set if key is builtin */
#define KEY_FLAG_ROOT_CAN_INVAL	9	/* set if key can be invalidated by root without permission */
#define KEY_FLAG_UID_KEYRING	10	/* set if key is a user or user session keyring */

	/* the key type and key description string
	 * - the desc is used to match a key against search criteria
	 * - it should be a printable string
	 * - eg: for krb5 AFS, this might be "afs@REDHAT.COM"
	 */
	union {
		struct keyring_index_key index_key;
		struct {
			struct key_type	*type;		/* type of key */
			char		*description;
		};
	};

	/* key data
	 * - this is used to hold the data actually used in cryptography or
	 *   whatever
	 */
	union {
		union key_payload payload;
		struct {
			/* Keyring bits */
			struct list_head name_link;
			struct assoc_array keys;
		};
	};
};

extern struct key *key_alloc(struct key_type *type,
			     const char *desc,
			     kuid_t uid, kgid_t gid,
			     const struct cred *cred,
			     key_perm_t perm,
			     unsigned long flags);


#define KEY_ALLOC_IN_QUOTA	0x0000	/* add to quota, reject if would overrun */
#define KEY_ALLOC_QUOTA_OVERRUN	0x0001	/* add to quota, permit even if overrun */
#define KEY_ALLOC_NOT_IN_QUOTA	0x0002	/* not in quota */
#define KEY_ALLOC_TRUSTED	0x0004	/* Key should be flagged as trusted */
#define KEY_ALLOC_UID_KEYRING	0x0010	/* allocating a user or user session keyring */

extern void key_revoke(struct key *key);
extern void key_invalidate(struct key *key);
extern void key_put(struct key *key);

static inline struct key *__key_get(struct key *key)
{
	atomic_inc(&key->usage);
	return key;
}

static inline struct key *key_get(struct key *key)
{
	return key ? __key_get(key) : key;
}

static inline void key_ref_put(key_ref_t key_ref)
{
	key_put(key_ref_to_ptr(key_ref));
}

extern struct key *request_key(struct key_type *type,
			       const char *description,
			       const char *callout_info);

extern struct key *request_key_with_auxdata(struct key_type *type,
					    const char *description,
					    const void *callout_info,
					    size_t callout_len,
					    void *aux);

extern struct key *request_key_async(struct key_type *type,
				     const char *description,
				     const void *callout_info,
				     size_t callout_len);

extern struct key *request_key_async_with_auxdata(struct key_type *type,
						  const char *description,
						  const void *callout_info,
						  size_t callout_len,
						  void *aux);

extern int wait_for_key_construction(struct key *key, bool intr);

extern int key_validate(const struct key *key);

extern key_ref_t key_create_or_update(key_ref_t keyring,
				      const char *type,
				      const char *description,
				      const void *payload,
				      size_t plen,
				      key_perm_t perm,
				      unsigned long flags);

extern int key_update(key_ref_t key,
		      const void *payload,
		      size_t plen);

extern int key_link(struct key *keyring,
		    struct key *key);

extern int key_unlink(struct key *keyring,
		      struct key *key);

extern struct key *keyring_alloc(const char *description, kuid_t uid, kgid_t gid,
				 const struct cred *cred,
				 key_perm_t perm,
				 unsigned long flags,
				 struct key *dest);

extern int keyring_clear(struct key *keyring);

extern key_ref_t keyring_search(key_ref_t keyring,
				struct key_type *type,
				const char *description);

extern int keyring_add_key(struct key *keyring,
			   struct key *key);

extern struct key *key_lookup(key_serial_t id);

static inline key_serial_t key_serial(const struct key *key)
{
	return key ? key->serial : 0;
}

extern void key_set_timeout(struct key *, unsigned);

/*
 * The permissions required on a key that we're looking up.
 */
#define	KEY_NEED_VIEW	0x01	/* Require permission to view attributes */
#define	KEY_NEED_READ	0x02	/* Require permission to read content */
#define	KEY_NEED_WRITE	0x04	/* Require permission to update / modify */
#define	KEY_NEED_SEARCH	0x08	/* Require permission to search (keyring) or find (key) */
#define	KEY_NEED_LINK	0x10	/* Require permission to link */
#define	KEY_NEED_SETATTR 0x20	/* Require permission to change attributes */
#define	KEY_NEED_ALL	0x3f	/* All the above permissions */

static inline short key_read_state(const struct key *key)
{
	/* Barrier versus mark_key_instantiated(). */
	return smp_load_acquire(&key->state);
}

/**
 * key_is_positive - Determine if a key has been positively instantiated
 * @key: The key to check.
 *
 * Return true if the specified key has been positively instantiated, false
 * otherwise.
 */
static inline bool key_is_positive(const struct key *key)
{
	return key_read_state(key) == KEY_IS_POSITIVE;
}

static inline bool key_is_negative(const struct key *key)
{
	return key_read_state(key) < 0;
}

#define rcu_dereference_key(KEY)					\
	(rcu_dereference_protected((KEY)->payload.rcu_data0,		\
				   rwsem_is_locked(&((struct key *)(KEY))->sem)))

#define rcu_assign_keypointer(KEY, PAYLOAD)				\
do {									\
	rcu_assign_pointer((KEY)->payload.rcu_data0, (PAYLOAD));	\
} while (0)

#ifdef CONFIG_SYSCTL
extern struct ctl_table key_sysctls[];
#endif
/*
 * the userspace interface
 */
extern int install_thread_keyring_to_cred(struct cred *cred);
extern void key_fsuid_changed(struct task_struct *tsk);
extern void key_fsgid_changed(struct task_struct *tsk);
extern void key_init(void);

#else /* CONFIG_KEYS */

#define key_validate(k)			0
#define key_serial(k)			0
#define key_get(k) 			({ NULL; })
#define key_revoke(k)			do { } while(0)
#define key_invalidate(k)		do { } while(0)
#define key_put(k)			do { } while(0)
#define key_ref_put(k)			do { } while(0)
#define make_key_ref(k, p)		NULL
#define key_ref_to_ptr(k)		NULL
#define is_key_possessed(k)		0
#define key_fsuid_changed(t)		do { } while(0)
#define key_fsgid_changed(t)		do { } while(0)
#define key_init()			do { } while(0)

#endif /* CONFIG_KEYS */
#endif /* __KERNEL__ */
#endif /* _LINUX_KEY_H */
