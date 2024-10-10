#!/bin/bash

APP_PATH="/home/pi/ClimateView/wirelessTemp.py"
PID_FILE="/home/pi/ClimateView/wirelessTemp.pid"

start() {
    if [ -f $PID_FILE ]; then
        echo "The service is already running."
    else
        source "/home/pi/ClimateView/rf24/bin/activate"
        nohup python3 -u $APP_PATH &>> /home/pi/ClimateView/wirelessTemp.log &
        echo $! > $PID_FILE
        echo "Service started."
    fi
}

stop() {
    if [ -f $PID_FILE ]; then
        kill $(cat $PID_FILE)
        rm $PID_FILE
        echo "Service stopped."
    else
        echo "The service is not running."
    fi
}

restart() {
    stop
    start
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        restart
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
esac