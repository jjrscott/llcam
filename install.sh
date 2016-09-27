#/bin/bash

sudo service fastcam stop
sudo cp -pr build/fastcam_producer /usr/local/bin/fastcam_producer
sudo cp -pr build/fastcam_consumer /usr/lib/cgi-bin/fastcam
sudo cp -pr src/initd.sh /etc/init.d/fastcam
sudo update-rc.d fastcam defaults
sudo service fastcam start