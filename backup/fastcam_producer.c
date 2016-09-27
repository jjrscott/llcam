#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define SHM_ID "/mmap-test"
#define FRAME_COUNT 5
#define FRAME_DATA_SIZE (1<<17)
#define SLEEP_NANOS 500000000000L   // 1 micro

struct Frame
{
    size_t _length;	
    char _data[FRAME_DATA_SIZE];
};

struct RingBuffer
{
    size_t _wseq;
    char _pad2[64];

    struct Frame _buffer[FRAME_COUNT];
};

void
producerLoop()
{
    int size = sizeof( struct RingBuffer );
    umask(0);
    int fd = shm_open( SHM_ID, O_RDWR | O_CREAT, 0666 );
    ftruncate( fd, size+1 );

    // create shared memory area
    struct RingBuffer* rb = (struct RingBuffer*)mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    close( fd );

    // initialize our sequence numbers in the ring buffer
    size_t seq  = -1;
    rb->_wseq = seq;

    struct timespec tss;
    tss.tv_sec = 1;
    tss.tv_nsec = 0;//SLEEP_NANOS;
    
    while( 1 )
    {
    	seq = (seq + 1) % FRAME_COUNT;
    	
    	char path[256];
    	
    	FILE *fp; /*filepointer*/
    	
    	struct Frame* frm = &rb->_buffer[seq];
    	
    	
    	sprintf(path, "/var/www/html/stream-%02d.jpg", seq+1);

		fp = fopen(path,"rb"); /*open file*/
		
		if (fp)
		{
			fseek(fp, 0, SEEK_END); 
			frm->_length = ftell(fp);         /*calc the size needed*/
			fseek(fp, 0, SEEK_SET);
		
			fread(frm->_data, 1, frm->_length, fp);
		
			fclose(fp);
		}
		else
		{
			perror(path);
			return;
		}
		

    	
    	
    	
    	
    	
    	rb->_wseq = seq;
    
//     
//         // as long as the consumer isn't running behind keep producing
//         while( (rb->_wseq+1)%BUFFER_SIZE != rb->_rseq%BUFFER_SIZE )
//         {
//             // write the next entry and atomically update the write sequence number
//             struct Message* msg = &rb->_buffer[rb->_wseq%BUFFER_SIZE];
//             msg->_id = i++;
//             __sync_fetch_and_add( &rb->_wseq, 1 );
//         }

        // give consumer some time to catch up
        nanosleep( &tss, 0 );
    }
}


int
main( int argc, char** argv )
{
	producerLoop();
}