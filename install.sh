#/bin/bash

sudo service llcam stop
sudo killall --signal KILL -r llcam
sudo cp -pr build/llcam_audio_producer /usr/local/bin/llcam_audio_producer
sudo cp -pr build/llcam_video_producer /usr/local/bin/llcam_video_producer
sudo cp -pr build/llcam_audio_consumer /usr/lib/cgi-bin/llcam_audio_consumer
sudo cp -pr build/llcam_video_consumer /usr/lib/cgi-bin/llcam_video_consumer
sudo cp -pr src/initd.sh /etc/init.d/llcam
sudo cp -pr src/llcam.html /var/www/html/llcam
sudo update-rc.d llcam defaults
sudo service llcam start