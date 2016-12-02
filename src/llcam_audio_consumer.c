#include "llcam_common.h"

#include <stdio.h>
#include <stdlib.h>
extern char **environ;

#define SLEEP_NANOS 1000   // 1 micro

struct WAVFileHeader {
	char _opaque0[22];
	uint16_t channels;
	uint32_t samples_per_second;
	uint32_t bytes_per_second;
	uint16_t block_align;
	uint16_t bits_per_sample;
	char _opaque1[8];
};

struct AUFileHeader {
	char magic[4];
	uint32_t hdr_size;
	uint32_t data_size;
	uint32_t encoding;
	uint32_t sample_rate;
	uint32_t channels;
};

#define AUDIO_FILE_ENCODING_LINEAR_16 (3) 

void
consumerLoop()
{
    int size = sizeof( struct AudioSharedMemory );
    int fd = shm_open( AUDIO_SHARED_MEMORY_IDENTIFIER, O_RDONLY, 0666 );
    if( fd == -1 ) {
        perror( "argh!!!" ); return;
    }

    // lookup producers shared memory area
    struct AudioSharedMemory* shared_memory = (struct AudioSharedMemory*)mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
    close( fd );

    // initialize our sequence numbers in the ring buffer
    size_t last_frame_index = -1;

    struct timespec tss;
    tss.tv_sec = 0;
    tss.tv_nsec = SLEEP_NANOS;
    
    printf("Content-Type: audio/basic\n\n");
    	
	struct AUFileHeader audio_header;
	
	memcpy(audio_header.magic, ".snd", 4);
	
	audio_header.hdr_size = htobe32(sizeof(struct AUFileHeader));
	audio_header.data_size = htobe32(0xffffffff);
	audio_header.encoding = htobe32(AUDIO_FILE_ENCODING_LINEAR_16);
   audio_header.sample_rate = htobe32(shared_memory->samples_per_second);
   audio_header.channels = htobe32(shared_memory->channels);

    
    
    fwrite(&audio_header, 1, sizeof(struct AUFileHeader), stdout);


    while( 1 )
    {
        // while there is data to consume
        while( last_frame_index != shared_memory->_last_frame_index )
        {
        	last_frame_index = shared_memory->_last_frame_index;
        	
        	struct AudioFrame* audio_frame = shared_memory->_frames + last_frame_index;

        	fwrite(audio_frame->_data, 1, audio_frame->_length, stdout);
        }

        // wait for more data
        nanosleep( &tss, 0 );
    }
}

int
main( int argc, char** argv )
{
// 	char *range = getenv("HTTP_RANGE");
// 
// 	printf("Accept-Ranges: bytes\n");
// 	
// 	if (range)
// 	{
// 		printf("Status: 206\n");
// 		printf("Content-Type: audio/wav\n");
// 		printf("Content-Range: %s/400000\n", range);
// 		printf("Content-Length: 2\n");
// 		printf("\n");
// 
// 		printf("RI");
// 
// 	}
// 	else
// 	{
// 		printf("Content-Length: 400000\n");
// 	}
		consumerLoop();
}