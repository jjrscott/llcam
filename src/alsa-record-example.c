/* 
  A Minimal Capture Program

  This program opens an audio interface for capture, configures it for
  stereo, 16 bit, 44.1kHz, interleaved conventional read/write
  access. Then its reads a chunk of random data from it, and exits. It
  isn't meant to be a real program.

  From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html

  Fixes rate and buffer problems

  sudo apt-get install libasound2-dev
  gcc -o alsa-record-example -lasound alsa-record-example.c && ./alsa-record-example hw:0
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdint.h>
#include <asm/byteorder.h>

struct AuFile {
   uint8_t magic[4];       /* magic number */
   uint32_t hdr_size;    /* size of this header */
   uint32_t data_size;   /* length of data (optional) */
   uint32_t encoding;    /* data encoding format */
   uint32_t sample_rate; /* samples per second */
   uint32_t channels;    /* number of interleaved channels */
   uint8_t data[];
};


void main (int argc, char *argv[])
{
  int i;
  int err;
  struct AuFile *buffer;
  int buffer_frames = 4410;
  unsigned int rate = 44100;
  unsigned int channels = 1;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_BE;

  if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             argv[1],
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

  unsigned int buffer_length = buffer_frames * snd_pcm_format_width(format) / 8 * channels;

  buffer = malloc(sizeof(struct AuFile) + buffer_length);

  fprintf(stdout, "buffer allocated\n");
  
  FILE *fp;
  fp = fopen(argv[2], "w");
  
   buffer->magic[0] ='.';       /* magic number */
   buffer->magic[1] ='s';       /* magic number */
   buffer->magic[2] ='n';       /* magic number */
   buffer->magic[3] ='d';       /* magic number */
   buffer->hdr_size = __cpu_to_be32(24);    /* size of this header */
   buffer->data_size = __cpu_to_be32(buffer_length);   /* length of data (optional) */
   buffer->encoding = __cpu_to_be32(3);    /* data encoding format */
   buffer->sample_rate = __cpu_to_be32(rate); /* samples per second */
   buffer->channels = __cpu_to_be32(channels);    /* number of interleaved channels */

  for (i = 0; i < 1; ++i) {
    if ((err = snd_pcm_readi (capture_handle, buffer->data, buffer_frames)) != buffer_frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
               err, snd_strerror (err));
      exit (1);
    }
    fwrite(buffer->data, 1, buffer_length, fp);
    fprintf(stdout, "read %d done (%d)\n", i, err);
  }

  fclose(fp);
  free(buffer);

  fprintf(stdout, "buffer freed\n");
	
  snd_pcm_close (capture_handle);
  fprintf(stdout, "audio interface closed\n");

  exit (0);
}
