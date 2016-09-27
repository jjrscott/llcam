#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define SLEEP_NANOS 1000   // 1 micro

#define SHM_ID "/mmap-test"
#define FRAME_COUNT 5
#define FRAME_DATA_SIZE (1<<17)

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
consumerLoop()
{
    int size = sizeof( struct RingBuffer );
    int fd = shm_open( SHM_ID, O_RDWR, 0666 );
    if( fd == -1 ) {
        perror( "argh!!!" ); return;
    }

    // lookup producers shared memory area
    struct RingBuffer* rb = (struct RingBuffer*)mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

    // initialize our sequence numbers in the ring buffer
    size_t seq = -1;

    struct timespec tss;
    tss.tv_sec = 0;
    tss.tv_nsec = SLEEP_NANOS;
    
    printf("Content-Type: multipart/x-mixed-replace; boundary=endofsection\n\n");

    while( 1 )
    {
        // while there is data to consume
        while( seq != rb->_wseq )
        {
        	seq = rb->_wseq;
        	
        	struct Frame* frm = &rb->_buffer[seq];

        	printf("Content-Type: image/jpeg\n\n");
//         	printf("X-FastCam-Seq: %d\n\n", seq);
        	
        	fwrite(frm->_data, 1, frm->_length, stdout);
        	
        	printf("\n--endofsection\n");
        }

        // wait for more data
        nanosleep( &tss, 0 );
    }
}

int
main( int argc, char** argv )
{
	consumerLoop();
}