#include <linux/module.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>

MODULE_LICENSE("GPL v2");

static int etherip_open(struct net_device *dev) {
    netif_start_queue(dev);
    return 0;
}
static int etherip_close(struct net_device *dev) {
    netif_stop_queue(dev);
    return 0;
}
static netdev_tx_t etherip_xmit(struct sk_buff *skb, struct net_device *dev)
{
    /* IPv6 Header, EtherIP Header追加 */
    struct sk_buff *new_skb;
    struct iphdr *iph;
    dev_kfree_skb(skb);

    new_skb = alloc_skb(1500 + LL_RESERVED_SPACE(dev), GFP_ATOMIC);

    if (!new_skb) {
        return NETDEV_TX_BUSY;
    }

    skb_put(new_skb, sizeof(struct ipv6hdr));
    ip6h = ipv6_hdr(new_skb);
    ip6h->version     = 6;
    ip6h->priority    = 0;
    ip6h->flow_lbl    = 0;
    ip6h->payload_len = htons(l2_len);
    ip6h->nexthdr     = 97;
    ip6h->hop_limit   = 64;
    
    return NETDEV_TX_OK;
}

static const struct net_device_ops etherip_netdev_ops = {
    .ndo_open       = etherip_open,
    .ndo_stop       = etherip_close,
    .ndo_start_xmit = etherip_xmit,
};

struct etherip_device {
    struct net_device *dev;
};

int setup_device(void) {
    struct etherip_device *priv;
    int err;
    struct net_device *dev;

    dev = alloc_netdev_mqs(sizeof(struct etherip_device), "etherip%d", NET_NAME_UNKNOWN, ether_setup, 1, 1);
    if (!dev) {
        return -ENOMEM;
    }

    priv = netdev_priv(dev);
    dev->netdev_ops = &etherip_netdev_ops;
    eth_hw_addr_random(dev);
    err = register_netdev(dev);

    if (err) {
        free_netdev(dev);
        return err;
    }
    return 0;
}

/* インストール時に実行 */
static int etherip_init(void) {
    int err;
    err = setup_device();
    if (err) {
        return err;
    }

    printk(KERN_ALERT "Hello World!\n");
    return 0;
}

/* アンインストール時に実行 */
static void etherip_exit(void) {
    printk(KERN_ALERT "Goodbye World!\n");
}

module_init(etherip_init);
module_exit(etherip_exit);