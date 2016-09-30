/* 
 * File:   buffer_demo.c
 * Author: Tasanakorn
 *
 * Created on May 22, 2013, 1:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <alsa/asoundlib.h>
#include <pthread.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

#define VIDEO_FPS 10
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

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

typedef struct {
    int width;
    int height;
    MMAL_COMPONENT_T *camera;
    MMAL_COMPONENT_T *encoder;
    MMAL_PORT_T *camera_video_port;
    MMAL_POOL_T *camera_video_port_pool;
    MMAL_PORT_T *encoder_input_port;
    MMAL_POOL_T *encoder_input_pool;
    MMAL_PORT_T *encoder_output_port;
    MMAL_POOL_T *encoder_output_pool;
    float fps;
    
    struct RingBuffer* ring_buffer;
   	size_t ring_buffer_seq;
} PORT_USERDATA;

void *background_audio_loop( void * );

pthread_t start_background_audio_loop( PORT_USERDATA * );

static void camera_video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
//     fprintf(stderr, "> %s\n", __func__);
    static int frame_count = 0;
    static struct timespec t1;
    struct timespec t2;

    //fprintf(stderr, "INFO:%s\n", __func__);
    if (frame_count == 0) {
        clock_gettime(CLOCK_MONOTONIC, &t1);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
//      fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

    int d = t2.tv_sec - t1.tv_sec;

    MMAL_BUFFER_HEADER_T *new_buffer;
    MMAL_BUFFER_HEADER_T *output_buffer = 0;
    PORT_USERDATA *userdata = (PORT_USERDATA *) port->userdata;

    MMAL_POOL_T *pool = userdata->camera_video_port_pool;


//      fprintf(stderr, "- %s:%d\n", __func__, __LINE__);
     frame_count++;

    output_buffer = mmal_queue_get(userdata->encoder_input_pool->queue);

    //Try to some colors http://en.wikipedia.org/wiki/YUV
    int chrominance_offset = userdata->width * userdata->height;
    int v_offset = chrominance_offset / 4;
    int chroma = 0;
//      fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

    if (output_buffer) {
        mmal_buffer_header_mem_lock(buffer);
//              fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

        memcpy(output_buffer->data, buffer->data, buffer->length);
//              fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

//      fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

        output_buffer->length = buffer->length;
        mmal_buffer_header_mem_unlock(buffer);
        if (mmal_port_send_buffer(userdata->encoder_input_port, output_buffer) != MMAL_SUCCESS) {
            fprintf(stderr, "ERROR: Unable to send buffer \n");
        }
    } else {
        fprintf(stderr, "ERROR: mmal_queue_get (%d)\n", output_buffer);
    }

//      fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

    if (frame_count % 10 == 0) {
        // print framerate every n frame
        clock_gettime(CLOCK_MONOTONIC, &t2);
        float d = (t2.tv_sec + t2.tv_nsec / 1000000000.0) - (t1.tv_sec + t1.tv_nsec / 1000000000.0);
        float fps = 0.0;

        if (d > 0) {
            fps = frame_count / d;
        } else {
            fps = frame_count;
        }
        userdata->fps = fps;
//         fprintf(stderr, "  Frame = %d,  Framerate = %.1f fps \n", frame_count, fps);
    }

//      fprintf(stderr, "- %s:%d\n", __func__, __LINE__);

    mmal_buffer_header_release(buffer);

    // and send one back to the port (if still open)
    if (port->is_enabled) {
        MMAL_STATUS_T status;

        new_buffer = mmal_queue_get(pool->queue);

        if (new_buffer) {
            status = mmal_port_send_buffer(port, new_buffer);
        }

        if (!new_buffer || status != MMAL_SUCCESS) {
            fprintf(stderr, "Error: Unable to return a buffer to the video port\n");
        }
    }
//     fprintf(stderr, "< %s\n", __func__);
}

static void encoder_input_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
//     fprintf(stderr, "> %s\n", __func__);
    mmal_buffer_header_release(buffer);
//     fprintf(stderr, "< %s\n", __func__);
}

static void encoder_output_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
//     fprintf(stderr, "> %s\n", __func__);
    MMAL_BUFFER_HEADER_T *new_buffer;
    PORT_USERDATA *userdata = (PORT_USERDATA *) port->userdata;
    MMAL_POOL_T *pool = userdata->encoder_output_pool;
    //fprintf(stderr, "INFO:%s\n", __func__);

    mmal_buffer_header_mem_lock(buffer);
//     fwrite(buffer->data, 1, buffer->length, stdout);

// -------------------

    	userdata->ring_buffer_seq = (userdata->ring_buffer_seq + 1) % FRAME_COUNT;
    	
//     	fprintf(stdout, "output: %d %d\n", userdata->ring_buffer_seq, buffer->length);

    	
    	struct Frame* ring_buffer_frame = userdata->ring_buffer->_buffer + userdata->ring_buffer_seq;
    	
    	ring_buffer_frame->_length = buffer->length;
        memcpy(ring_buffer_frame->_data, buffer->data, buffer->length);
    	
    	userdata->ring_buffer->_wseq = userdata->ring_buffer_seq;



// -------------------


    mmal_buffer_header_mem_unlock(buffer);

    mmal_buffer_header_release(buffer);
    if (port->is_enabled) {
        MMAL_STATUS_T status;

        new_buffer = mmal_queue_get(pool->queue);

        if (new_buffer) {
            status = mmal_port_send_buffer(port, new_buffer);
        }

        if (!new_buffer || status != MMAL_SUCCESS) {
            fprintf(stderr, "Unable to return a buffer to the video port\n");
        }
    }
//     fprintf(stderr, "< %s\n", __func__);
}

int fill_port_buffer(MMAL_PORT_T *port, MMAL_POOL_T *pool) {
    int q;
    int num = mmal_queue_length(pool->queue);

    for (q = 0; q < num; q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
        if (!buffer) {
            fprintf(stderr, "Unable to get a required buffer %d from pool queue\n", q);
        }

        if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS) {
            fprintf(stderr, "Unable to send a buffer to port (%d)\n", q);
        }
    }
}

int setup_camera(PORT_USERDATA *userdata) {
    fprintf(stderr, "> %s\n", __func__);
    MMAL_STATUS_T status;
    MMAL_COMPONENT_T *camera = 0;
    MMAL_ES_FORMAT_T *format;
    MMAL_PORT_T * camera_video_port;
    MMAL_POOL_T * camera_video_port_pool;

    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: create camera %x\n", status);
        return -1;
    }
    userdata->camera = camera;
    userdata->camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];

    camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];


    {
        MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
            { MMAL_PARAMETER_CAMERA_CONFIG, sizeof (cam_config)},
            .max_stills_w = 1280,
            .max_stills_h = 720,
            .stills_yuv422 = 0,
            .one_shot_stills = 1,
            .max_preview_video_w = VIDEO_WIDTH,
            .max_preview_video_h = VIDEO_HEIGHT,
            .num_preview_video_frames = 3,
            .stills_capture_circular_buffer_height = 0,
            .fast_preview_resume = 0,
            .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
        };
        mmal_port_parameter_set(camera->control, &cam_config.hdr);
    }

    // Setup camera video port format
    format = camera_video_port->format;
    format->encoding = MMAL_ENCODING_I420;
    format->encoding_variant = MMAL_ENCODING_I420;
    format->es->video.width = VIDEO_WIDTH;
    format->es->video.height = VIDEO_HEIGHT;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = VIDEO_WIDTH;
    format->es->video.crop.height = VIDEO_HEIGHT;
    format->es->video.frame_rate.num = VIDEO_FPS;
    format->es->video.frame_rate.den = 1;

    camera_video_port->buffer_size = format->es->video.width * format->es->video.height * 12 / 8;
    camera_video_port->buffer_num = 2;

    fprintf(stderr, "INFO:camera video buffer_size = %d\n", camera_video_port->buffer_size);
    fprintf(stderr, "INFO:camera video buffer_num = %d\n", camera_video_port->buffer_num);

    status = mmal_port_format_commit(camera_video_port);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to commit camera video port format (%u)\n", status);
        return -1;
    }

    camera_video_port_pool = (MMAL_POOL_T *) mmal_port_pool_create(camera_video_port, camera_video_port->buffer_num, camera_video_port->buffer_size);
    userdata->camera_video_port_pool = camera_video_port_pool;
    camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *) userdata;


    status = mmal_port_enable(camera_video_port, camera_video_buffer_callback);

    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to enable camera video port (%u)\n", status);
        return -1;
    }

    status = mmal_component_enable(camera);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to enable camera (%u)\n", status);
        return -1;
    }


    fill_port_buffer(userdata->camera_video_port, userdata->camera_video_port_pool);

    if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS) {
        printf("%s: Failed to start capture\n", __func__);
    }

    fprintf(stderr, "INFO: camera created\n");
    return 0;
}

int setup_encoder(PORT_USERDATA *userdata) {
    fprintf(stderr, "> %s\n", __func__);
    MMAL_STATUS_T status;
    MMAL_COMPONENT_T *encoder = 0;

    MMAL_PORT_T *encoder_input_port = NULL, *encoder_output_port = NULL;
    MMAL_POOL_T *encoder_input_port_pool;
    MMAL_POOL_T *encoder_output_port_pool;

    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to create preview (%u)\n", status);
        return -1;
    }

    encoder_input_port = encoder->input[0];
    encoder_output_port = encoder->output[0];
    userdata->encoder_input_port = encoder_input_port;
    userdata->encoder_output_port = encoder_input_port;

    mmal_format_copy(encoder_input_port->format, userdata->camera_video_port->format);
    encoder_input_port->buffer_size = encoder_input_port->buffer_size_recommended;
    encoder_input_port->buffer_num = 2;


    mmal_format_copy(encoder_output_port->format, encoder_input_port->format);

    encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;
    encoder_output_port->buffer_num = 2;
    // Commit the port changes to the input port 
    status = mmal_port_format_commit(encoder_input_port);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to commit encoder input port format (%u)\n", status);
        return -1;
    }

    // Only supporting H264 at the moment
    encoder_output_port->format->encoding = MMAL_ENCODING_MJPEG;
    encoder_output_port->format->bitrate = 2000000;

    encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;

    if (encoder_output_port->buffer_size < encoder_output_port->buffer_size_min) {
        encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
    }

    encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;

    if (encoder_output_port->buffer_num < encoder_output_port->buffer_num_min) {
        encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;
    }


    // Commit the port changes to the output port    
    status = mmal_port_format_commit(encoder_output_port);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to commit encoder output port format (%u)\n", status);
        return -1;
    }

    fprintf(stderr, " encoder input buffer_size = %d\n", encoder_input_port->buffer_size);
    fprintf(stderr, " encoder input buffer_num = %d\n", encoder_input_port->buffer_num);

    fprintf(stderr, " encoder output buffer_size = %d\n", encoder_output_port->buffer_size);
    fprintf(stderr, " encoder output buffer_num = %d\n", encoder_output_port->buffer_num);

    encoder_input_port_pool = (MMAL_POOL_T *) mmal_port_pool_create(encoder_input_port, encoder_input_port->buffer_num, encoder_input_port->buffer_size);
    userdata->encoder_input_pool = encoder_input_port_pool;
    encoder_input_port->userdata = (struct MMAL_PORT_USERDATA_T *) userdata;
    status = mmal_port_enable(encoder_input_port, encoder_input_buffer_callback);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to enable encoder input port (%u)\n", status);
        return -1;
    }
    fprintf(stderr, "INFO:Encoder input pool has been created\n");


    encoder_output_port_pool = (MMAL_POOL_T *) mmal_port_pool_create(encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size);
    userdata->encoder_output_pool = encoder_output_port_pool;
    encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *) userdata;

    status = mmal_port_enable(encoder_output_port, encoder_output_buffer_callback);
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Error: unable to enable encoder output port (%u)\n", status);
        return -1;
    }
    fprintf(stderr, "INFO:Encoder output pool has been created\n");    

    fill_port_buffer(encoder_output_port, encoder_output_port_pool);

    fprintf(stderr, "INFO:Encoder has been created\n");
    fprintf(stderr, "< %s\n", __func__);
    return 0;
}

int setup_producer(PORT_USERDATA *userdata) {

    int size = sizeof( struct RingBuffer );
    umask(0);
    int fd = shm_open( SHM_ID, O_RDWR | O_CREAT, 0666 );
    ftruncate( fd, size+1 );

    // create shared memory area
    userdata->ring_buffer = (struct RingBuffer*)mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    close( fd );

    // initialize our sequence numbers in the ring buffer
    userdata->ring_buffer_seq = -1;
    userdata->ring_buffer->_wseq = userdata->ring_buffer_seq;

    return 0;
}

int main(int argc, char** argv) {

    PORT_USERDATA userdata;
    MMAL_STATUS_T status;


    memset(&userdata, 0, sizeof (PORT_USERDATA));

    userdata.width = VIDEO_WIDTH;
    userdata.height = VIDEO_HEIGHT;
    userdata.fps = 0.0;

    fprintf(stderr, "VIDEO_WIDTH : %i\n", userdata.width );
    fprintf(stderr, "VIDEO_HEIGHT: %i\n", userdata.height );
    fprintf(stderr, "VIDEO_FPS   : %i\n",  VIDEO_FPS);
    fprintf(stderr, "Running...\n");

    bcm_host_init();
    
    if (1 && setup_producer(&userdata) != 0) {
        fprintf(stderr, "Error: setup producer %x\n", status);
        return -1;
    }

    if (1 && setup_camera(&userdata) != 0) {
        fprintf(stderr, "Error: setup camera %x\n", status);
        return -1;
    }


    if (1 && setup_encoder(&userdata) != 0) {
        fprintf(stderr, "Error: setup encoder %x\n", status);
        return -1;
    }
    
    start_background_audio_loop(&userdata);
    


    while (1) {
		usleep(30000);
    }

    return 0;
}

pthread_t start_background_audio_loop(PORT_USERDATA *userdata) {
     pthread_t thread1;
     int  iret1, iret2;

    /* Create independent threads each of which will execute function */

     iret1 = pthread_create( &thread1, NULL, background_audio_loop, userdata);
     if(iret1)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }

	return thread1;
}

void *background_audio_loop(void *context)
{
  PORT_USERDATA *userdata = context;

  int i;
  int err;
  char *buffer;
  int buffer_frames = 4410;
  unsigned int rate = 44100;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	
  char * name = "plughw:1";

  if ((err = snd_pcm_open (&capture_handle, name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             name,
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface opened\n");
		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params allocated\n");
				 
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params initialized\n");
	
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params access setted\n");
	
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params format setted\n");
	
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    exit (1);
  }
	
  fprintf(stdout, "hw_params rate setted\n");

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params channels setted\n");
	
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params setted\n");
	
  snd_pcm_hw_params_free (hw_params);

  fprintf(stdout, "hw_params freed\n");
	
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface prepared\n");

  buffer = malloc(buffer_frames * snd_pcm_format_width(format) / 8 * 2);

  fprintf(stdout, "buffer allocated\n");

  for (i = 0; i < 10; ++i) {
    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
               err, snd_strerror (err));
      exit (1);
    }
    fprintf(stdout, "read %d done (%d)\n", i, err);
  }

  free(buffer);

  fprintf(stdout, "buffer freed\n");
	
  snd_pcm_close (capture_handle);
  fprintf(stdout, "audio interface closed\n");
}
