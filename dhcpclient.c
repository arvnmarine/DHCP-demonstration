
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <net/if.h> 
#include <unistd.h>

#define MAXLINE 80

char clientMAC[13];

int main(int argc, char *argv[])
{
	struct  sockaddr_in servaddr;
	int	sockfd;
	int	servlen;
	int	ntimes;
	int	n;

	char	buf[100];
	char	ptime[100];

	//handle input format
	if (argc != 3) {
		printf("Usage: client <address> <port>\n");
		exit(-1);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket error");
		exit(-1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(atoi(argv[2]));

	servlen = sizeof(servaddr);

	
	
	  /*getMAC*/
	struct ifreq s;
  strcpy(s.ifr_name, "eth0");
  if (0 == ioctl(sockfd, SIOCGIFHWADDR, &s)) {
    int i;
	/*
    for (i = 0; i < 6; ++i)
      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
    puts("\n");
	*/
    
  }
	
	
	while (1) {
		
		
		
		
		
		sprintf(buf, "D%02x%02x%02x%02x%02x%02x", (unsigned char) s.ifr_addr.sa_data[0],
		(unsigned char) s.ifr_addr.sa_data[1],(unsigned char) s.ifr_addr.sa_data[2],
		(unsigned char) s.ifr_addr.sa_data[3],(unsigned char) s.ifr_addr.sa_data[4],
		(unsigned char) s.ifr_addr.sa_data[5]);
			
			
		/* DHCP Discover */
		if (sendto(sockfd, (void *)buf, sizeof(buf), 0, 
				  (struct sockaddr *)&servaddr, servlen) < 0) {
			perror("sendto error");
			exit(-1);
		}
		printf("Client: I am discovering\n");
		sleep(1);
		
		int count;
		for (count=0;count<2;count++){//run 2 times to capture the Ack from server if server assign an address
			if (recvfrom(sockfd, ptime, MAXLINE, 0, (struct sockaddr *)&servaddr, &servlen)>=0) {  
				if (ptime[0] == 'O') {//DHCP Offer
					printf("Client: I received an offer address 192.168.1.%c%c%c\n", ptime[1], ptime[2], ptime[3]);
					
					/*DHCP Request*/
					sprintf(buf, "R%02x%02x%02x%02x%02x%02x%c%c%c", (unsigned char) s.ifr_addr.sa_data[0],
						(unsigned char) s.ifr_addr.sa_data[1],(unsigned char) s.ifr_addr.sa_data[2],
						(unsigned char) s.ifr_addr.sa_data[3],(unsigned char) s.ifr_addr.sa_data[4],
						(unsigned char) s.ifr_addr.sa_data[5],ptime[1],ptime[2],ptime[3]);
					if (sendto(sockfd, (void *)buf, sizeof(buf), 0, 
							  (struct sockaddr *)&servaddr, servlen) < 0) {
						perror("sendto error");
						exit(-1);
					}
					printf("Client: Hey server, can you give me the address 192.168.1.%c%c%c ? \n", ptime[1], ptime[2], ptime[3]);
					
				} else if ((ptime[0] == 'A')){//DHCP Ack
					
						//check if this is the right Ack for this client
						char a[13]; char b[13];
						sprintf(a, "%02x%02x%02x%02x%02x%02x", (unsigned char) s.ifr_addr.sa_data[0],
						(unsigned char) s.ifr_addr.sa_data[1],(unsigned char) s.ifr_addr.sa_data[2],
						(unsigned char) s.ifr_addr.sa_data[3],(unsigned char) s.ifr_addr.sa_data[4],
						(unsigned char) s.ifr_addr.sa_data[5]);
						memcpy(b, &ptime[1], 12 );
						int x; x=1;
						int i;
						for (i=0;i<12;i++){
							if (a[i]!=b[i]) {
								x=0;
								break;
							}
						}
						if (x==1){
							printf("Client: Yay, I have address 192.168.1.%c%c%c, thank you server\n", ptime[13],ptime[14],ptime[15]);
							exit(0);
						}
					
				}
			}	
			sleep(1);
		}
			
			
		
	}

}
