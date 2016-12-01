#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "llcam_common.h"

#define SLEEP_NANOS 1000   // 1 micro

void
consumerLoop()
{
    int size = sizeof( struct VideoSharedMemory );
    int fd = shm_open( VIDEO_SHARED_MEMORY_IDENTIFIER, O_RDONLY, 0666 );
    if( fd == -1 ) {
        perror( "argh!!!" ); return;
    }

    // lookup producers shared memory area
    struct VideoSharedMemory* shared_memory = (struct VideoSharedMemory*)mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
    close( fd );

    // initialize our sequence numbers in the ring buffer
    size_t last_frame_index = -1;

    struct timespec tss;
    tss.tv_sec = 0;
    tss.tv_nsec = SLEEP_NANOS;
    
    printf("Content-Type: multipart/x-mixed-replace; boundary=endofsection\n\n");

    while( 1 )
    {
        // while there is data to consume
        while( last_frame_index != shared_memory->_last_frame_index )
        {
        	last_frame_index = shared_memory->_last_frame_index;
        	
        	struct VideoFrame* video_frame = shared_memory->_frames + last_frame_index;

        	printf("Content-Type: image/jpeg\n\n");
//         	printf("X-llcam-Seq: %d\n\n", seq);
        	
        	fwrite(video_frame->_data, 1, video_frame->_length, stdout);
        	
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