/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>

/*ETHERNET DEFINITION*/

/*define my MAC address*/
#define DEST_MAC0	0x00
#define DEST_MAC1	0x1c
#define DEST_MAC2	0xc0
#define DEST_MAC3	0x96
#define DEST_MAC4	0x01
#define DEST_MAC5	0x68
/*ethernet type*/
#define ETHER_TYPE	ETHERTYPE_IP
/*physical ethernet port name*/
#define DEFAULT_IF	"eth0"
/*size of the array in which store data*/
#define BUF_SIZ		1024

/*XILLYBUS DEFINITION*/
/*all "xilly_*" variables and functions are refered
to PCIe (thus FPGA) comunication*/

/*variable that ensure that PCIe's write file descriptor
is correctly initialized*/
static bool xilly_write_initted = false;

/*PCIe's write file descriptor*/
static int  xilly_fd_write = -1;


/**
 * @XILLY_DEV_WRITE: /path/fd_name of the file descriptor
 *
 * xilly_write_init: check if @XILLY_DEV_WRITE is already opened
 *		     (thanks to xilly_write_initted variable)
 *		     if not, open it in write only mode
 *
 * Returns:	1 --> if all is going well
 *	       -1 --> if something appens
 */
 
int xilly_write_init (const char *XILLY_DEV_WRITE)
{
    if (xilly_write_initted)
    {
        fprintf (stderr, "Only once you need to initialize the write process\n");
        return -1;
    }

    xilly_fd_write = open (XILLY_DEV_WRITE, O_WRONLY);

    if (xilly_fd_write < 0)
    {
        fprintf (stderr, "Error opening write device %s:%s\n", XILLY_DEV_WRITE, strerror(errno));
        return -1;
    }

    xilly_write_initted = true;
    return 1;
}

/**
 * @void
 *
 * xilly_write_deinit:	deinitialize write file descriptor
 *
 * Returns:	1 --> if all is going well
 *	       -1 --> if fd is already deinit.
 */
int xilly_write_deinit ()
{
    xilly_write_initted = false;

    if (xilly_fd_write)
	{
        close (xilly_fd_write);
        return 1;
    }
    return -1;
}
/*
* @IOVEC *IOV: structure where data and data's lenth from etherent are stored
*
* @OUT_FD: file descriptor 
*
* @COUNT: data's lenth
*
* do_recvbuff: create a pipe -> send buff to the input pipe -> send the output pipe to out_fd
*
* Returns :     number of bytes sent --> if all is going well
*		-1 --> something wrong happend
*
*/
ssize_t do_recvbuff(int out_fd, const struct iovec *iov, size_t count) {
    ssize_t bytes, bytes_sent, bytes_in_pipe;
    size_t total_bytes_sent = 0;
	int pipefd[2];
	
	if ( pipe(pipefd) < 0 ) {
    perror("pipe");
    exit(EXIT_FAILURE);
	}
	printf("I'm sending : %ld\n", count);
    // Vmsplice the data from buff into the pipe
    while (total_bytes_sent < count) {
        if ((bytes_sent = vmsplice(pipefd[1], iov ,1,0)) <= 0) {
			
            if (errno == EINTR || errno == EAGAIN) {                
                continue;
            }
            perror("splice1");
            return -1;
        }
        else
		printf("Vmsplice is working!!!\n");

        // Splice the data from the pipe into out_fd
        bytes_in_pipe =bytes_sent;

        while (bytes_in_pipe > 0) {

            if ((bytes = splice(pipefd[0], NULL, out_fd, NULL, bytes_in_pipe,
                    SPLICE_F_MORE | SPLICE_F_MOVE)) <= 0) {
						
                if (errno == EINTR || errno == EAGAIN) {
					
                    // Interrupted system call/try again
                    // Just skip to the top of the loop and try again
                    continue;
                }
                printf("%s\n", strerror(errno));
                perror("splice2");
                return -1;
            }
            bytes_in_pipe -= bytes;
        }
        total_bytes_sent += bytes_sent;
    }
    return total_bytes_sent;
}

int main(int argc, char *argv[])
{
	char sender[INET6_ADDRSTRLEN];
	int sockfd, ret, i;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
	char ifName[IFNAMSIZ];
	struct iovec iov;
	
	int sent_to_xilly;
	
	if (xilly_write_init ("/dev/xillybus_write_32") < 0 )
    {
        fprintf (stderr, "Error initializing xilly write");
        return -1;
    }
	
	
	/* Get physical port name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		strcpy(ifName, DEFAULT_IF);

	/* Header structures */
	struct ether_header *eh = (struct ether_header *) buf;
	struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (buf +sizeof(struct iphdr) + sizeof(struct ether_header));
	
	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");	
		return -1;
	}
	
	
	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	
	
//repeat:	
	//call function recvfrom: recieve from socket data and store it in buf
	while((numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL))>0){

		printf("listener: Waiting to recvfrom...\n");
		
		printf("listener: got packet %lu bytes\n", numbytes);

        
	/* Check the packet is for me */
	if (eh->ether_dhost[0] == DEST_MAC0 &&
			eh->ether_dhost[1] == DEST_MAC1 &&
			eh->ether_dhost[2] == DEST_MAC2 &&
			eh->ether_dhost[3] == DEST_MAC3 &&
			eh->ether_dhost[4] == DEST_MAC4 &&
			eh->ether_dhost[5] == DEST_MAC5) {
			
		iov.iov_base=buf;
		iov.iov_len=numbytes;

		if ((sent_to_xilly=do_recvbuff(xilly_fd_write,&iov,numbytes))<0)
			perror("redirect buffer:");
		else
		    printf("Sent to xilly: %d\n",sent_to_xilly);
	
		/* Get source IP */
		((struct sockaddr_in *)&their_addr)->sin_addr.s_addr = iph->saddr;
		inet_ntop(AF_INET, &((struct sockaddr_in*)&their_addr)->sin_addr, sender, sizeof sender);

		/* Look up my device IP addr if possible */
		strncpy(if_ip.ifr_name, ifName, IFNAMSIZ-1);
		if (ioctl(sockfd, SIOCGIFADDR, &if_ip) >= 0) { /* if we can't check then don't */
			printf("Source IP: %s\n My IP: %s\n", sender, 
					inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
			/* ignore if I sent it */
			if (strcmp(sender, inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr)) == 0)	{
				printf("but I sent it :(\n");
				ret = -1;
			}
		}

		/* UDP payload length */
		ret = ntohs(udph->len) - sizeof(struct udphdr);

		/* Print packet */
		printf("\tData:");
		for (i=0; i<numbytes; i++){
			 
			 printf("%02x:", buf[i]);}
			 printf("\n");
			printf("Correct destination MAC address\n");
		
	} else {
		printf("Wrong destination MAC: %x:%x:%x:%x:%x:%x\n",
						eh->ether_dhost[0],
						eh->ether_dhost[1],
						eh->ether_dhost[2],
						eh->ether_dhost[3],
						eh->ether_dhost[4],
						eh->ether_dhost[5]);
		ret = -1;

	}

	}
	xilly_write_deinit();
	close(sockfd);
	return ret;
}
