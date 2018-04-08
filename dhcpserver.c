/*
	Done by Nam Ho
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>


char clientMAC[13];
char arr[256][12];

int getAdd(){
	int  i;
	for (i=2;i<256;i++){
		if ((arr[i][0]=='\0')) {return i;}
		if ((arr[i][0]==clientMAC[0]) && (arr[i][1]==clientMAC[1]) &&
			(arr[i][2]==clientMAC[2]) && (arr[i][3]==clientMAC[3]) &&
			(arr[i][4]==clientMAC[4]) && (arr[i][5]==clientMAC[5]) &&
			(arr[i][6]==clientMAC[6]) && (arr[i][7]==clientMAC[7]) &&
			(arr[i][8]==clientMAC[8]) && (arr[i][9]==clientMAC[9]) &&
			(arr[i][10]==clientMAC[10]) && (arr[i][11]==clientMAC[11]))
			return i;
	}
	return 0;
}

int cmp(int i){
	int j;
	for (j=0;j<12;j++){
		if (arr[i][j]!=clientMAC[j])
			return 0;
	}
	return 1;
}

int passive_udp(char *address, int port);

int main(int argc, char *argv[])
{


	int sockfd;
	int clilen;
	struct sockaddr_in cliaddr;

	time_t tnow;	
	char ptime[100];
	char buf[100];

	//handle input format
	if (argc != 3) {
		printf("Usage: server <address> <port> \n");
		exit(-1);
	}

	sockfd = passive_udp(argv[1], atoi(argv[2]));

	while (1) { 
		clilen = sizeof(cliaddr);
		if (recvfrom(sockfd, buf, sizeof(buf), 0,  
			     (struct sockaddr *)&cliaddr,
			     &clilen) < 0) {
			perror("recvfrom error");
			exit(-1);
		}
		
		memcpy( clientMAC, &buf[1], 12 );
		
		int ip_add;
		ip_add = getAdd();
		if (buf[0] == 'D') {//DHCP Discover
			
			printf ("Server: I see that client %s is dicovering \n",clientMAC);
			if (ip_add == 0) { // server is full
				printf("Server says: I am full now, free up slot by reboot me\n");
			} else if (arr[ip_add][0] == '\0') {//found an address to assign, reserve address .0 and .1
				
				
				
				////DHCP Offer
				sprintf(ptime,"O%d%d%d", 
					(ip_add/100) % 10,(ip_add/10) % 10,(ip_add % 10));
				
				
				sendto(sockfd, (const void*)ptime, strlen(ptime), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
				printf("Server: I offer address 192.168.1.%d%d%d\n",
					(ip_add/100) % 10,(ip_add/10) % 10,(ip_add % 10));
			} else if (arr[ip_add][0] != '\0') {//client has been assigned an address before, duplicated error
				
				printf("Server: Client %s already have an address, this is a duplicated error\n",clientMAC);
				
				//DHCP Ack
				sprintf(ptime,"A%s%d%d%d", clientMAC,
					(ip_add/100) % 10,(ip_add/10) % 10,(ip_add % 10));
				
				sendto(sockfd, (const void*)ptime, strlen(ptime), 0,(struct sockaddr *)&cliaddr, sizeof(cliaddr));
				
				
				//update the address/MAC bank
				int i;
				for (i=0;i<12;i++) {
					arr[ip_add][i]=clientMAC[i];
				}
				
				
				printf("Server: Hey client %s, I assigned you to address 192.168.1.%d%d%d\n",clientMAC,
					(ip_add/100) % 10,(ip_add/10) % 10,(ip_add % 10));
					
			}
		} else if (buf[0] == 'R') {//DHCP Request
			char reqIpStr[3];
			memcpy( reqIpStr, &buf[13], 3 );
			int reqIp;
			reqIp = 100*((int)reqIpStr[0]-48)
					+10*((int)reqIpStr[1]-48)
					+((int)reqIpStr[2]-48);
			
			printf("Server: I see that client %s is requesting address 192.168.1.%c%c%c\n",clientMAC,reqIpStr[0],reqIpStr[1],reqIpStr[2]);
			
			if ((arr[reqIp][0] == '\0') || (cmp(reqIp)==1)) {//check to see if the address is still available
				
				
				
				
				//DHCP Ack, let client know they are assigned new address or they had an address and this is duplicated error
				sprintf(ptime,"A%s%d%d%d", clientMAC,
					(ip_add/100) % 10,(ip_add/10) % 10,(ip_add % 10));
				
				sendto(sockfd, (const void*)ptime, strlen(ptime), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
				
				int i;
				for (i=0;i<12;i++) {
					arr[ip_add][i]=clientMAC[i];
				}
				
				printf("Server: Hey client %s, I assigned you to address 192.168.1.%d%d%d\n",clientMAC,
					(ip_add/100) % 10,(ip_add/10) % 10,(ip_add % 10));
			} else {// Ip address is taken by another client
				
				printf("Server: Sorry client %s, this address has been issued to another client\n",clientMAC);
				
			}
		}
		
		
	}

}

int passive_udp(char *address, int port)
{

	int sockfd, n;
        struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket error");
		return(-1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(address);
	servaddr.sin_port = htons(port);


	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind error");
		return(-1);
	}

	return(sockfd);
}	
