#include <linux/time.h> 
#include <linux/version.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <net/route.h>
#include <linux/inet.h>
#include <linux/kernel.h>
#include <linux/init.h>  
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>  
#include <linux/netfilter_ipv4.h>
#include <linux/net.h>
#include <net/ip.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <asm/byteorder.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct nf_hook_ops nfho;


int addrswap(struct sk_buff *skb)
{
	 struct sk_buff *nskb, *new_skb;
	  struct iphdr *iph = NULL; 
	  struct udphdr *udph = NULL;
        
        struct iphdr *new_iph = NULL; //new ip头
	  struct udphdr *new_udph = NULL;//new udp头
	  u_long sip = in_aton("192.168.55.111");   //源主机
   	  u_long dip = in_aton("10.1.0.100");   //对方主机
	printk("bbbbbbbbbbb\n");
        if(skb == NULL)
                return NF_ACCEPT;

	nskb = pskb_copy(skb, GFP_ATOMIC);
		iph = ip_hdr(nskb);
	 	udph = udp_hdr(nskb);
	
        if(udph->dest == htons(9999))
        {			
		
		if(pskb_expand_head(skb,sizeof(struct udphdr)+sizeof(struct iphdr),0,GFP_ATOMIC))
			return 0;

		skb_push(skb, sizeof(struct udphdr));
		skb_reset_transport_header(skb);

		new_udph = udp_hdr(skb);
		
		printk("cccccccccc\n");
		//重新封装原UDP头部
		memset(new_udph, 0, sizeof(struct udphdr));
		new_udph->source = udph->source;
		new_udph->dest = udph->dest;
		nskb->csum = 0;
		new_udph->len = htons(ntohs(iph->tot_len) + sizeof(struct udphdr)); //原来IP数据包长度(作为新数据)+udp头部长度
		new_udph->check = 0;


		//重新封装IP头部
		skb_push(skb, sizeof(struct iphdr));
		skb_reset_network_header(skb);

		new_iph = ip_hdr(skb);

		memset(new_iph, 0, sizeof(struct iphdr));
	      new_iph->version = 4;
	      new_iph->ihl = iph->ihl;
	      new_iph->frag_off = iph->frag_off;
	      new_iph->protocol = IPPROTO_UDP;
	      new_iph->id = iph->id;
	      new_iph->tos = iph->tos;
	      new_iph->saddr = iph->saddr;
	      new_iph->daddr = dip;
	      new_iph->ttl = iph->ttl;
	      new_iph->tot_len = htons(ntohs(skb->len));             
	      new_iph->check = 0;
	      new_iph->check = ip_fast_csum((unsigned char *)new_iph, new_iph->ihl);
	    
	      skb->csum = skb_checksum(skb, new_iph->ihl*4, skb->len - new_iph->ihl*4, 0);
		new_udph->check = csum_tcpudp_magic(sip, iph->daddr, skb->len - iph->ihl * 4, IPPROTO_UDP, skb->csum);
		
		printk("djsflakejf\n");
		kfree_skb(nskb);
		    return NF_ACCEPT;		
        }	
        return NF_ACCEPT;
}

//钩子函数，注意参数格式与开发环境源码树保持一致
unsigned int hook_func(const struct nf_hook_ops *ops, 
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	printk("aaaaaaaaaaaaa\n");
	return addrswap(skb);
	
}

static int __init hook_init(void)
{
    nfho.hook = hook_func;//关联对应处理函数
    nfho.hooknum = NF_INET_PRE_ROUTING;//ipv4的第一个hook
    nfho.pf = PF_INET;//ipv4，所以用这个
    nfho.priority = NF_IP_PRI_FIRST;//优先级，第一顺位

    nf_register_hook(&nfho);//注册

    return 0;
}
static void __exit hook_exit(void)
{
    nf_unregister_hook(&nfho);//注销
}

module_init(hook_init);
module_exit(hook_exit);
