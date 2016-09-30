#!/usr/bin/perl

foreach my $command (<DATA>)
{
	print qx($command);
}


__DATA__
gcc -o build/fastcam_producer src/fastcam_producer.c -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/mmal/ -lmmal -L/opt/vc/lib/ -lmmal_core -lmmal_util -lvcos -lpthread -lbcm_host -lrt
gcc -o build/fastcam_consumer src/fastcam_consumer.c -lrt

gcc -o build/alsa-record-example src/alsa-record-example.c -I/usr/include/dssi -lasound -lpthread
#gcc -o build/pthread1 src/pthread1.c -lpthread

