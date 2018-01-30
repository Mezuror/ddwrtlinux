/* linux/net/inet/arp.h */
#ifndef _ARP_H
#define _ARP_H

#include <linux/if_arp.h>
#include <linux/hash.h>
#include <net/neighbour.h>

#ifdef CONFIG_KERNEL_ARP_SPOOFING_PROTECT
#define MAX_ARP_SPOOFING_TABLE 256
struct st_ip_mac_table
{
	__be32 szIPaddr;
	unsigned char szMac[ETH_ALEN];
};
#endif
extern struct neigh_table arp_tbl;

static inline u32 arp_hashfn(const void *pkey, const struct net_device *dev, u32 *hash_rnd)
{
	u32 key = *(const u32 *)pkey;
	u32 val = key ^ hash32_ptr(dev);

	return val * hash_rnd[0];
}

static inline struct neighbour *__ipv4_neigh_lookup_noref(struct net_device *dev, u32 key)
{
	if (dev->flags & (IFF_LOOPBACK | IFF_POINTOPOINT))
		key = INADDR_ANY;

	return ___neigh_lookup_noref(&arp_tbl, neigh_key_eq32, arp_hashfn, &key, dev);
}

static inline struct neighbour *__ipv4_neigh_lookup(struct net_device *dev, u32 key)
{
	struct neighbour *n;

	rcu_read_lock_bh();
	n = __ipv4_neigh_lookup_noref(dev, key);
	if (n && !atomic_inc_not_zero(&n->refcnt))
		n = NULL;
	rcu_read_unlock_bh();

	return n;
}

void arp_init(void);
int arp_ioctl(struct net *net, unsigned int cmd, void __user *arg);
void arp_send(int type, int ptype, __be32 dest_ip,
	      struct net_device *dev, __be32 src_ip,
	      const unsigned char *dest_hw,
	      const unsigned char *src_hw, const unsigned char *th);
int arp_mc_map(__be32 addr, u8 *haddr, struct net_device *dev, int dir);
void arp_ifdown(struct net_device *dev);

struct sk_buff *arp_create(int type, int ptype, __be32 dest_ip,
			   struct net_device *dev, __be32 src_ip,
			   const unsigned char *dest_hw,
			   const unsigned char *src_hw,
			   const unsigned char *target_hw);
void arp_xmit(struct sk_buff *skb);

#ifdef CONFIG_KERNEL_ARP_SPOOFING_PROTECT
int arp_spoofing_protect(struct sk_buff *skb);
#endif

#endif	/* _ARP_H */
