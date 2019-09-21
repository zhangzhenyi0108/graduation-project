#pragma once

#define NETLINK_TEST     30
#define MSG_LEN            125
#define USER_PORT        100
#define ADD_ENTRY	1
#define DEL_ENTRY	2
#define PRINT_MAPTABLE 3

typedef struct netlink_msg {
	int type;
	unsigned int local_1;
	unsigned int local_2;
}netlink_msg;



