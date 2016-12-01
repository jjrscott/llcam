#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define AUDIO_SHARED_MEMORY_IDENTIFIER "/fastcam_audio"

#define AUDIO_FRAME_COUNT 128
#define AUDIO_FRAME_DATA_SIZE (1<<12)

struct AudioFrame
{
    size_t _length;
    char _data[AUDIO_FRAME_DATA_SIZE];
};

struct AudioSharedMemory
{
    size_t channels;
    size_t samples_per_second;
    size_t bits_per_sample;
    size_t _last_frame_index;
    struct AudioFrame _frames[AUDIO_FRAME_COUNT];
};

#define VIDEO_SHARED_MEMORY_IDENTIFIER "/fastcam_video"

#define VIDEO_FRAME_COUNT 5
#define VIDEO_FRAME_DATA_SIZE (1<<17)

struct VideoFrame
{
    size_t _length;
    char _data[VIDEO_FRAME_DATA_SIZE];
};

struct VideoSharedMemory
{
    size_t _last_frame_index;
    struct VideoFrame _frames[VIDEO_FRAME_COUNT];
};

