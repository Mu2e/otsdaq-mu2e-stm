/*
	Simple udp server
*/
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUFLEN 10000	//Max length of buffer
#define PORT 51872	//The port on which to listen for incoming data

void die(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in si_me, si_other;
	
	int s, i, slen = sizeof(si_other) , recv_len;
	unsigned short buf[BUFLEN];
        int j = 0;
	
	//create a UDP socket
	s=socket(AF_INET, SOCK_DGRAM,0 );
	if ((s=socket(AF_INET, SOCK_DGRAM,0 )) == -1) //IPPROTO_UDP
	{
		die("socket");
	}
       
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	//	si_me.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("192.168.34.6");
	si_me.sin_addr.s_addr = inet_addr("192.168.34.6");
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		die("bind");
	}
	
	//keep listening for data
	//	while(j<3)
	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);
		
                //slen = sizeof(si_other);
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			die("recvfrom()");
		}
		
		//print details of the client/peer and the data received
		printf("Received packet %d from %s:%d length:%d\n",j, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port),recv_len);
		for (i=19;i < (recv_len/2) - 1; ++i)
                   {
                     if (buf[i+1] - buf[i] != 1) { printf ("Index,Data: %d,%#x- %d,%#x   ",i,buf[i],i+1,buf[i+1] ); }
                //               
                   }
                printf("Data: %#x,%#x,%#x,%#x\n" , buf[19], buf[20], buf[21], buf[22]);
		
		//now reply the client with the same data
		//if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		//{
		//	die("sendto()");
		//}
                j+=1;
	}

	close(s);
	return 0;
}
