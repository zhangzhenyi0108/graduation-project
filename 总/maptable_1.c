#include <linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include <linux/slab.h>
#include <linux/jhash.h>
#include <linux/inet.h>


#include "maptable_1.h"


MODULE_LICENSE("Dual BSD/GPL");

static unsigned int in4_hashmask_local = HASH_MASK;
static unsigned int in4_hashrnd = 7963;

unsigned int in4_hashcode_local(unsigned int addr)  //计算hash值
{
	return (jhash_1word(addr, in4_hashrnd) &in4_hashmask_local)%MAX_HLIST_COUNT;
}



//初始化哈希表
void init_hlists(void) {
	int i = 0;
	for (i = 0; i < MAX_HLIST_COUNT; i++) {
		INIT_HLIST_HEAD(&maptable_hlist[i]);
	}
}


int del_entry(unsigned int local_1, unsigned int local_2) {
	unsigned int hash;
	struct maptable_store *pt;
	struct hlist_head *head;

	hash = in4_hashcode_local(local_1);
	//遍历
	head = &(maptable_hlist[hash]);

	hlist_for_each_entry(pt,  head, m_hlist) {
		if ((pt->localaddr_1 == local_1) && (pt->localaddr_2 == local_2)) {
			printk("Done\n");
			__hlist_del(&(pt->m_hlist));
			kfree(pt);
			pt = NULL;
			return 1;
			break;
		}
	}
	printk("Not Found!\n");
	return 0;
}
EXPORT_SYMBOL(del_entry);


void add_entry(unsigned int local_1, unsigned int local_2) {
	unsigned int hash;
	struct maptable_store *pt;
	struct hlist_head *head;

	hash = in4_hashcode_local(local_1);
	
	head = &(maptable_hlist[hash]);
	hlist_for_each_entry(pt, head, m_hlist) {
		if (pt->localaddr_1 == local_1) {
			del_entry(pt->localaddr_1, pt->localaddr_2);
			add_entry(local_1, local_2);
			return;
		}
	}

	pt = kmalloc(sizeof(maptable_store), GFP_KERNEL);
	if (!pt) {
		printk("kmalloc pt error!\n");
		return;
	}

	memset(pt, 0, sizeof(maptable_store));
	pt->localaddr_1 = local_1;
	pt->localaddr_2 = local_2;
	INIT_LIST_HEAD(&pt->m_list); 
	INIT_HLIST_NODE(&pt->m_hlist);

	hlist_add_head(&(pt->m_hlist), head);
	
	printk("Finished!\n");
	return;
}
EXPORT_SYMBOL(add_entry);


struct maptable_store* lookup_maptable(unsigned int local_1, unsigned int *local_2) {
	unsigned int hash;
	struct maptable_store *pt;
	struct hlist_head *head;

	hash = in4_hashcode_local(local_1);
	head = &(maptable_hlist[hash]);
	hlist_for_each_entry(pt, head, m_hlist) {
		if ((pt->localaddr_1 == local_1)) {
			*local_2 = pt->localaddr_2;
			return pt;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(lookup_maptable);

void print_ntoh(unsigned int a, unsigned int b)
{
	unsigned char *p1 = (unsigned char *)&a;
	unsigned char *p2 = (unsigned char *)&b;
	printk("AID1:     %d.%d.%d.%d\tAID2:      %d.%d.%d.%d\n", p1[0] & 0xff,
			p1[1] & 0xff, p1[2] & 0xff, p1[3] & 0xff, p2[0] & 0xff,
			p2[1] & 0xff, p2[2] & 0xff, p2[3] & 0xff);
}

void print_hlist(void) {
	struct maptable_store *pt;
	struct hlist_head *head;
	int i = 0;
	printk("================Maptable List================\n");

	for (i = 0; i < MAX_HLIST_COUNT; i++) {
		head = &(maptable_hlist[i]);
		hlist_for_each_entry(pt, head, m_hlist) {
			//printk("AID1:  %d\tAID2:  %d\n", (pt->localaddr_1), (pt->localaddr_2));
			print_ntoh(pt->localaddr_1, pt->localaddr_2);
		}
	}
	printk("==============================================\n");
}
EXPORT_SYMBOL(print_hlist);

void clean_maptable(void) {
	int i;
	struct maptable_store *pt;
	struct hlist_node *cur, *next;

	for (i = 0; i < MAX_HLIST_COUNT; i++) {
		hlist_for_each_entry_safe(pt, cur, &(maptable_hlist[i]), m_hlist) {
			__hlist_del(&(pt->m_hlist));
			kfree(pt);
		}
	}
	printk("=========FREE DONE=========\n");
}


void init_system(void)
{
	init_hlists();

	add_entry((unsigned int)333, (unsigned int)444);
	add_entry((unsigned int)123, (unsigned int)234);
	add_entry((unsigned int)111, (unsigned int)555);
	add_entry((unsigned int)in_aton("192.168.87.131"), (unsigned int)in_aton("2.3.2.3"));
	
}




/*模块初始化接口*/
static int __init maptable_init(void)
{
	printk("Maptable,Initializing...\n");

	init_system();
	
	unsigned int i;

	if(lookup_maptable((unsigned int)111, &i)!= NULL){
		printk("find: AID2: %d\n",i);
	}else{
		printk("Not Found!\n");
	}
		
	print_hlist();

	return 0;
}

static void __exit maptable_exit(void)
{
	clean_maptable();
	printk("Maptable Terminated!\n");
}

module_init(maptable_init);
module_exit(maptable_exit);
