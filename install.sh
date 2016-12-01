#/bin/bash

sudo service fastcam stop
sudo killall --signal KILL -r fastcam
sudo cp -pr build/fastcam_audio_producer /usr/local/bin/fastcam_audio_producer
sudo cp -pr build/fastcam_video_producer /usr/local/bin/fastcam_video_producer
sudo cp -pr build/fastcam_audio_consumer /usr/lib/cgi-bin/fastcam_audio_consumer
sudo cp -pr build/fastcam_video_consumer /usr/lib/cgi-bin/fastcam_video_consumer
sudo cp -pr src/initd.sh /etc/init.d/fastcam
sudo cp -pr src/fastcam.html /var/www/html/fastcam
sudo update-rc.d fastcam defaults
sudo service fastcam start