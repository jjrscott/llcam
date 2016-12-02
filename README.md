
`llcam` is a aims to provide a low latency camera server with the minimum of fuss. In order to accomplish this splits audio and video is handled completely seperately; this means that audio and video may be out of sync. However, given that the latency of either streem is less than 100ms, the two can't deviate much.

Each type (audio or video) is split into two programs:

- `llcam_[audio|video]_producer`: This process copies data from the hardware into a shared ring buffer. There is a single producer process (for each type) running at any one time.
- `llcam_[audio|video]_consumer`: This is a CGI script (written in C) started by Apache in response to a web client request. It copies date from the ring buffer to stdout which Apache then sends on to the web client.

Currently `llcam_video_consumer` outputs JPEGs in a form that can be directly read displayed by HTML's `<img>` tag while `llcam_audio_consumer` outputs a continuous AU audio file.

### Build and Install

	./make.pl 
	./install.sh
	
The _stream_ can then be accessed using a normal web browser at `http://<Your Raspberry Pi's IP>/llcam`.

### Requirements

- libraspberrypi-dev
- Apache
