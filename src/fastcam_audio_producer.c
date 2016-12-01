/* 
 * File:   buffer_demo.c
 * Author: Tasanakorn
 *
 * Created on May 22, 2013, 1:52 PM
 */

#include "fastcam_common.h"
#include <alsa/asoundlib.h>

int main(int argc, char** argv) {

    int size = sizeof( struct AudioSharedMemory );
    umask(0);
    int fd = shm_open( AUDIO_SHARED_MEMORY_IDENTIFIER, O_RDWR | O_CREAT, 0666 );
    ftruncate( fd, size+1 );

    // create shared memory area
    struct AudioSharedMemory* shared_memory = (struct AudioSharedMemory*)mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    close( fd );

	shared_memory->_last_frame_index = -1;

  int i;
  int err;
  struct AuFile *buffer;
  unsigned int rate = 44100;
  unsigned int channels = 1;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_BE;
	
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

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, channels)) < 0) {
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
  
  shared_memory->channels = channels;
	shared_memory->samples_per_second = rate;
	shared_memory->bits_per_sample = snd_pcm_format_width(format);
  

   int buffer_frames = AUDIO_FRAME_DATA_SIZE * 8 / snd_pcm_format_width(format) / channels;

  fprintf(stdout, "snd_pcm_format_width: %d\n", snd_pcm_format_width(format));
  fprintf(stdout, "buffer_frames: %d\n", buffer_frames);

   while (1) {
   	size_t frame_index = (shared_memory->_last_frame_index + 1) % AUDIO_FRAME_COUNT;
	
    struct AudioFrame* audio_frame = shared_memory->_frames + frame_index;
	audio_frame->_length = AUDIO_FRAME_DATA_SIZE;
   
    if ((err = snd_pcm_readi (capture_handle, audio_frame->_data, buffer_frames)) != buffer_frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
               err, snd_strerror (err));
      exit (1);
    }
    
    shared_memory->_last_frame_index = frame_index;
    
//     fprintf(stdout, "frame index %d\n", frame_index);
    
    
// 	usleep(1000);
  }

//   free(buffer);

  fprintf(stdout, "buffer freed\n");
	
  snd_pcm_close (capture_handle);
  fprintf(stdout, "audio interface closed\n");
}
