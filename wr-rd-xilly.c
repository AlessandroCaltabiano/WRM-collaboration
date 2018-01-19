#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <termios.h>


#define IN_DATA_FILE "dummy_test.txt"
#define OUT_DATA_FILE "fpga_out.txt"
int MAX_DATA_SIZE=0;
static bool xilly_write_initted = false;
static bool xilly_read_initted = false;
static int  xilly_fd_write = -1;
static int  xilly_fd_read = -1;



/**
 * xilly_write_init:
 *
 * @XILLY_DEV_WRITE: a #const
 *
 * Returns:
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

/*
* xilly_read_init:
*
*@XILLY_DEV_READ: a#const
*
*
*
*Returns:
*/


int xilly_read_init(const char *XILLY_DEV_READ)
{
	if (xilly_read_initted)
	{
	 fprintf(stderr,"Only once you need to initialize read process\n");
	 return -1;
	}
	xilly_fd_read=open(XILLY_DEV_READ,O_RDONLY);
	if (xilly_fd_read<0)
	{
	 fprintf(stderr,"Error opening read device %s:%s\n",XILLY_DEV_READ,strerror(errno));
	return -1;
	}
	xilly_read_initted= true;
	return 1;
}




void * read_xilly (void * dev)
{
	FILE *Outfp;
	int rc=0;
	int count=0;
	Outfp=fopen(OUT_DATA_FILE,"w");
	int *fd_read=(int *)dev;
	unsigned int *read_buff= (unsigned int*) malloc(sizeof(unsigned int));
	unsigned int dummy_buff;
	
	
	struct timespec start,finish;
	
	double elapsed;
	
	
	clock_gettime(CLOCK_MONOTONIC,&start);
	
	while(count<MAX_DATA_SIZE)
	{
		rc=read(*fd_read,read_buff,4);	
		//printf("bytes read %d step %d value %u\n",rc,count,*read_buff);
		
		fprintf(Outfp,"%u\n",*read_buff);
		if (*read_buff==dummy_buff)
		{
			printf(" break at step %d\n",count);
			break;
		}
		
		dummy_buff=*read_buff;
		count+=1;
		
	}
	
	clock_gettime(CLOCK_MONOTONIC,&finish);
	
	
	elapsed=(finish.tv_sec-start.tv_sec);
	elapsed+=(finish.tv_nsec-start.tv_nsec)/1000000000.0;
	printf("%f\n",elapsed);	
	printf("%d\t %lu\n",MAX_DATA_SIZE,sizeof(read_buff));

	fclose(Outfp);
	return NULL;
}

void * write_xilly (void * dev)
{
	FILE *Infp;
	FILE *tempfp;
	int wc;
	int *fd_write=(int *)dev;
	Infp=fopen(IN_DATA_FILE,"r");
	tempfp=fopen("temp_data_w_file.txt","w");
	unsigned int *buff= (unsigned int *) malloc(sizeof(unsigned int)*MAX_DATA_SIZE);
	int count_temp = 0;
	
	while(!feof(Infp))
	{
		count_temp++;
		fscanf(Infp,"%u\n",buff);
		wc=write(*fd_write,buff,4);
		//write(*fd_write,buff,4);
		fprintf(tempfp,"%d\n",*buff);
		
		if (count_temp%1000 == 0)
		{
		
			printf("%d\n",wc);
			printf("count temp is %d\n", count_temp);
		}
		else{ 
		
			continue;
		}
		
	}
	
	
	
	fprintf(tempfp,"*********************\n");
	fprintf(tempfp,"%d\n",MAX_DATA_SIZE);
	

	

	fclose(tempfp);
	fclose(Infp);
	return NULL;
	 
}


/**
 * xilly_write_deinit:
 *
 * Returns:
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

int xilly_read_deinit()
{
 xilly_read_initted = false;

 if(xilly_fd_read)
 {
  close(xilly_fd_read);
  return 1;
  } 
  return -1;
}


int main(int argc, char **argv)
{

	pthread_t write_pth;
	pthread_t read_pth; 
	FILE *Infp;
	
	
	Infp=fopen(IN_DATA_FILE,"r");
	char c;
	int data_line=0;
	for (c=getc(Infp);c!=EOF;c=getc(Infp))
	{
		if (c=='\n')
			data_line=data_line+1;
	}
	MAX_DATA_SIZE=data_line;
	rewind(Infp);
	fclose(Infp);
	printf("%d\n",MAX_DATA_SIZE);
	
    if(xilly_read_init("/dev/xillybus_read_32")<0)
   {
    fprintf(stderr,"Error initializing xilly read\n");
    return -1;
    }
	
    if (xilly_write_init ("/dev/xillybus_write_32") < 0 )
    {
        fprintf (stderr, "Error initializing xilly write");
        return -1;
    }
    

	if(pthread_create(&read_pth,NULL,read_xilly,&xilly_fd_read))
	{
		fprintf(stderr, "Error creating read thread\n");
		return 1;
		}

	if(pthread_create(&write_pth,NULL,write_xilly,&xilly_fd_write))
	{
		fprintf(stderr, "Error creating write thread\n");
		return 1;
		}
		
	
	
   if(pthread_join(write_pth, NULL)) {

		fprintf(stderr, "Error joining write thread\n");
		return 2;

	}
	if(pthread_join(read_pth, NULL)) {

		fprintf(stderr, "Error joining read thread\n");
		return 2;

	}
	
	

out:
    xilly_write_deinit();
    xilly_read_deinit();

    return 0;
}
