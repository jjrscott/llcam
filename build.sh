#/bin/bash

gcc -o build/fastcam_producer src/fastcam_producer.c -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/mmal/ -lmmal -L/opt/vc/lib/ -lmmal_core -lmmal_util -lvcos -lpthread -lbcm_host -lrt
gcc -o build/fastcam_consumer src/fastcam_consumer.c -lrt
