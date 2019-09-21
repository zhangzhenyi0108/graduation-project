#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/inet.h>

#include "handlemsg.h"
#include "maptable_1.h"


MODULE_LICENSE("Dual BSD/GPL");

struct sock *nlsk = NULL;
extern struct net init_net;
extern void add_entry(unsigned int local_1, unsigned int local_2);
extern int del_entry(unsigned int local_1, unsigned int local_2);
extern void print_hlist(void);



int send_usrmsg(char *pbuf, uint16_t len)
{
	struct sk_buff *nl_skb;
	struct nlmsghdr *nlh;

	int ret;

	/* 创建sk_buff 空间 */
	nl_skb = nlmsg_new(len, GFP_ATOMIC);
	if (!nl_skb)
	{
		printk("netlink alloc failure\n");
		return -1;
	}

	/* 设置netlink消息头部 */
	nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
	if (nlh == NULL)
	{
		printk("nlmsg_put failaure \n");
		nlmsg_free(nl_skb);
		return -1;
	}

	/* 拷贝数据发送 */
	
	memcpy(nlmsg_data(nlh), pbuf, len);
	ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);

	return ret;
}

int send_maptable(struct hlist_head* pbuf, uint16_t len)
{
	struct sk_buff *nl_skb;
	struct nlmsghdr *nlh;

	int ret;

	/* 创建sk_buff 空间 */
	nl_skb = nlmsg_new(len, GFP_ATOMIC);
	if (!nl_skb)
	{
		printk("netlink alloc failure\n");
		return -1;
	}

	/* 设置netlink消息头部 */
	nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
	if (nlh == NULL)
	{
		printk("nlmsg_put failaure \n");
		nlmsg_free(nl_skb);
		return -1;
	}


	/* 拷贝数据发送 */
	memcpy(NLMSG_DATA(nlh), pbuf, len);
	ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);

	return ret;
}


static void netlink_rcv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct netlink_msg* nlmsg = NULL;
	char* kmsg = "successful";
	int type;
	int len;
	
	//char *kmsg = "hello users!!!";
	nlh = nlmsg_hdr(skb);
	if (nlh->nlmsg_len < sizeof(struct nlmsghdr)) {
		printk( "UnCorrupt netlink message.\n");
		return;
	}
	len = nlh->nlmsg_len - NLMSG_LENGTH(0);//宏NLMSG_LENGTH(len)用于计算数据部分长度为len时实际的消息长度。

	nlmsg = (struct netlink_msg *)NLMSG_DATA(nlh);
	type = nlmsg->type;
	switch (type) {
		case ADD_ENTRY:
			add_entry(nlmsg->local_1, nlmsg->local_2);	
			break;
		case DEL_ENTRY:
			del_entry(nlmsg->local_1, nlmsg->local_2);
			break;
		case PRINT_MAPTABLE:
			print_hlist();
			printk("%d\n",maptable_hlist);
			//send_maptable(maptable_hlist, sizeof(struct hlist_head)*1024);
			printk("%d\n",kmsg);
			send_usrmsg(kmsg, sizeof(char)*11);
			break;
		default:
			break;
	}


//	send_usrmsg(kmsg, strlen(kmsg));

}

struct netlink_kernel_cfg cfg = {
	.input = netlink_rcv_msg, /* set recv callback */
};

int test_netlink_init(void)
{
	/* create netlink socket */
	nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
	if (nlsk == NULL)
	{
		printk("netlink_kernel_create error !\n");
		return -1;
	}
	printk("test_netlink_init\n");
	
	return 0;
}

void test_netlink_exit(void)
{
	if (nlsk) {
		netlink_kernel_release(nlsk); /* release ..*/
		nlsk = NULL;
	}
	printk("test_netlink_exit!\n");
	
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);
