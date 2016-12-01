#!/bin/bash
# /etc/init.d/fastcam

### BEGIN INIT INFO
# Provides:          fastcam
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Example initscript
# Description:       This service is used to manage fastcam
### END INIT INFO


case "$1" in 
    start)
        echo "Starting fastcam"
        fastcam_audio_producer &
        fastcam_video_producer &
        ;;
    stop)
        echo "Stopping fastcam"
        sudo killall --signal KILL -r fastcam
        ;;
    *)
        echo "Usage: sudo service fastcam start|stop"
        exit 1
        ;;
esac

exit 0

