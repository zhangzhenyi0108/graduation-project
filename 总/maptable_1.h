#pragma once

#include<linux/list.h>
#define MAX_HLIST_COUNT 1024
#define HASH_MASK 1024*4-1

struct hlist_head maptable_hlist[MAX_HLIST_COUNT];

typedef struct maptable_store //存储表项
{
	unsigned int localaddr_1;   //标识1
	unsigned int localaddr_2;   //标识2

	struct list_head m_list;
	struct hlist_node m_hlist;
}maptable_store;
