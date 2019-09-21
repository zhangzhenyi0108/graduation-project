#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>


#include "handlemsg.h"
#include "listdemo.h"


#define NETLINK_TEST    30
#define MSG_LEN            125
#define MAX_PLOAD        125

typedef struct _user_msg_info
{
	struct nlmsghdr hdr;
	char  msg[MSG_LEN];
	struct hlist_head table_hlist[1024];
	
} user_msg_info;


void print(struct hlist_head* maptable_hlist) {
	struct maptable_store *pt;
	struct hlist_head *head;
	int i = 0;
	struct hlist_node *j;
	printf("================Maptable List================\n");
	for (i = 0; i < MAX_HLIST_COUNT; i++) {
		head = &(maptable_hlist[i]);
		hlist_for_each_entry(pt, head, m_hlist) {
			printf("AID1:  %d,\tAID2:  %d", pt->localaddr_1, pt->localaddr_2);
		}
	}
	printf("==============================================\n");

}

void print2(char* msg){
	printf("%s\n",msg);
}

int main(int argc, char *argv[])
{
	int skfd;
	int ret;
	user_msg_info u_info;
	socklen_t len;
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl saddr, daddr;
	struct netlink_msg *umsg = NULL;
	char type[4];
	unsigned int a;
	unsigned int b;
	char a0[20];
	char b0[20];
		
	umsg = (struct netlink_msg*)malloc(sizeof(struct netlink_msg));
	printf("请输入修改操作：\n");
	gets(type);

	if (!strcmp(type,"add")) {
		umsg->type = ADD_ENTRY;
		printf("请输入添加的条目：\n");
		scanf("%s",&a0);
		scanf("%s",&b0);
		inet_pton(AF_INET, a0, &a);
		inet_pton(AF_INET, b0, &b);
		umsg->local_1 = (unsigned int)a;
		umsg->local_2 = (unsigned int)b;
	}
	if (!strcmp(type,"del")) {
		umsg->type = DEL_ENTRY;
		printf("请输入删除的条目：\n");
		scanf("%s",&a0);
		scanf("%s",&b0);
		inet_pton(AF_INET, a0, &a);
		inet_pton(AF_INET, b0, &b);
		umsg->local_1 = (unsigned int)a;
		umsg->local_2 = (unsigned int)b;
	}
	if (!strcmp(type,"print")) {
		umsg->type = PRINT_MAPTABLE;
	}
	

	/* 创建NETLINK socket */
	skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if (skfd == -1)
	{
		perror("create socket error\n");
		return -1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.nl_family = AF_NETLINK; //AF_NETLINK
	saddr.nl_pid = 100;  //端口号(port ID) 
	saddr.nl_groups = 0;
	if (bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
	{
		perror("bind() error\n");
		close(skfd);
		return -1;
	}

	memset(&daddr, 0, sizeof(daddr));
	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0; // to kernel 
	daddr.nl_groups = 0;

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PLOAD));
	memset(nlh, 0, sizeof(struct nlmsghdr));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_type = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = saddr.nl_pid; //self port

	memcpy(NLMSG_DATA(nlh), umsg, sizeof(netlink_msg));

	ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
	if (!ret)
	{
		perror("sendto error\n");
		close(skfd);
		exit(-1);
	}


	//memset(&u_info, 0, sizeof(u_info));
  	memset(nlh,0,NLMSG_SPACE(1024));
	len = sizeof(struct sockaddr_nl);
	//ret = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
while(1){	
	ret = recvfrom(skfd, nlh, NLMSG_SPACE(1024), 0, (struct sockaddr *)&daddr, &len);
	
	if (!ret)
	{
		perror("recv form kernel error\n");
		close(skfd);
		exit(-1);
	}
print2((char*)NLMSG_DATA(nlh));
memset(nlh,0,NLMSG_SPACE(1024));
}
	//print((struct hlist_head*)NLMSG_DATA(nlh));
	//print(u_info.table_hlist);
	//printf("from kernel:%s\n", u_info.msg);
	//print2((char*)NLMSG_DATA(nlh));
	
	
	printf("--------------\n");	
	close(skfd);


	//free((void *)nlh);
	return 0;
}
