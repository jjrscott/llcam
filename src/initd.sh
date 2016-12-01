#!/bin/bash
# /etc/init.d/llcam

### BEGIN INIT INFO
# Provides:          llcam
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Example initscript
# Description:       This service is used to manage llcam
### END INIT INFO


case "$1" in 
    start)
        echo "Starting llcam"
        llcam_audio_producer &
        llcam_video_producer &
        ;;
    stop)
        echo "Stopping llcam"
        sudo killall --signal KILL -r llcam
        ;;
    *)
        echo "Usage: sudo service llcam start|stop"
        exit 1
        ;;
esac

exit 0

