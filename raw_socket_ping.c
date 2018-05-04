#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
//#include <linux/in.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
//#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_MAX 2048
#define calhex(sum) (uint16_t)((sum >> 16) & 0xffff) + (uint16_t)(sum & 0xffff)

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

struct timeval start;
struct timeval end;
struct sockaddr_in dest_addr;
pid_t pid; //进程序列号
int sockfd;
int datalen = 56;
char buffer[BUFFER_MAX];
char push_buffer[BUFFER_MAX];

void packet_push();
int set_icmp_pack(int process_id);
void packet_push();
void receive(char *buffer);
void packet_get();

int main(int argc, char *argv[]) // 输入个数
{
	unsigned long inaddr = 0;
	if (argc < 2)
	{
		printf("wrong input!\n");
		return 0;
	}
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)  //get icmp packets only
	{
		printf("error create raw socket\n");
		return -1;
	}
	//printf("fd is %d\n",sockfd);
	int size = 32 * 1024; // 避免缓冲区溢出。
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
	dest_addr.sin_family = AF_INET;

	if ((inaddr = inet_addr(argv[1])) == INADDR_NONE) // transfer to unsigned long 
	{
		printf("please input a correct IP\n");
		return 0;
	}
	else
	{
		dest_addr.sin_addr.s_addr = inaddr; //unsigned long
	}
	/*获取main的进程id,用于设置ICMP的标志符*/
	pid = getpid();
	packet_push();
	packet_get();
	double diet = (((double)end.tv_sec - start.tv_sec) * 1000000 + ((double)end.tv_usec - start.tv_usec)) / 1000;
	printf("ttl is %lf ms\n", diet);
	return 0;
}

int set_icmp_pack(int process_id)
{
	struct icmp *my_icmp;
	struct timeval *tval;
	int packsize = 0;
	my_icmp = (struct icmp *)push_buffer;
	my_icmp->icmp_type = ICMP_ECHO;
	my_icmp->icmp_code = 0;
	my_icmp->icmp_cksum = 0;
	my_icmp->icmp_seq = process_id;
	my_icmp->icmp_id = pid;
	packsize = 8 + datalen; // 8 means protocal-len
	tval = (struct timeval *)my_icmp->icmp_data;
	gettimeofday(tval, NULL);

	int len = packsize;
	unsigned int sum = 0;
	uint16_t *addr = (uint16_t *)push_buffer;
	while (len > 1)
	{
		sum += (*addr) & 0xffff;
		addr++;
		len = len - 2;
	}
	if (len == 1)
	{
		uint8_t *mid = (uint8_t *)addr;
		sum += (((uint16_t)(*mid)) << 8); // 8bits are solved as the high 16bits 
	}

	/*unsigned int pause_sum = calhex(sum);
	while(pause_sum&0xffff0000  != 0)
	{
		pause_sum = calhex(pause_sum);
	}
	my_icmp->icmp_cksum = ~pause_sum;*/

	my_icmp->icmp_cksum = ~((uint16_t)((sum >> 16) & 0xffff) + (uint16_t)(sum & 0xffff));
	return packsize;
}

void packet_push()
{
	gettimeofday(&start, NULL); //time counting is beginning
	//printf("start : %ld.%ld\n",start.tv_sec,start.tv_usec);

	int pack_size = set_icmp_pack(0);
	if (sendto(sockfd, push_buffer, pack_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
	{
		printf("fail to send the packet!\n");
	}
	else
	{
		printf("success to send the packet!\n");
	}
}

void packet_get()
{
	int n_read;
	//printf("start : %ld.%ld\n",start.tv_sec,start.tv_usec);
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		printf("error create raw socket\n");
		exit(-1);
	}

	//gettimeofday(&end, NULL);
	n_read = recvfrom(sockfd, buffer, 2048, 0, NULL, NULL);
	if (n_read < 46)
	{
		printf("error when recv msg \n");
		exit(-1);
	}

	gettimeofday(&end, NULL);
	//printf("end : %ld.%ld\n",end.tv_sec,end.tv_usec);

	receive(buffer);
	return;
}

void receive(char *buffer)
{
	char *ip_head, *icmp_head;
	unsigned char *p;
	unsigned short len = 0;
	ip_head = buffer;
	p = ip_head + 12;
	printf("from: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	printf("  to: %d.%d.%d.%d\n", p[4], p[5], p[6], p[7]);
}
