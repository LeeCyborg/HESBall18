#!/bin/bash
# HESBall Team Score Board

LOG_FILE="/home/debian/hes-ball-game.log"

SIGN_WD="/opt/LEDscape"
SIGN_CONFIG="matrix12x2.config"
SIGN_BIN_MAIN="./bin/hes-sign"
SIGN_BIN_DEBUG_OUT="./bin/sign-debug-out"

ARDUINO_STTY_FLAGS="-brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts"
## Add extra, maybe not nessiccary flag for `stty`
#ARDUINO_STTY_FLAGS="-icrnl $ARDUINO_STTY_FLAGS"
ARDUINO_SERIAL_RATE="9600"

MSG_ARDUINO_NONE="No Arduino plugged"
MSG_ARDUINO_WAIT="Waiting for handshake from Arduino"
MSG_ARDUINO_SCAN="Initial scan for Arduino"
MSG_ARDUINO_CONNECT="Arduino found! Connecting..."
MSG_ARDUINO_READY_FAIL="Search terminated without finding the pattern"

READY_STRING_ARDUINO="Arduino READY!"
READY_STRING_BBONE="BBone READY!"

HES_LOG() {
  echo "$1" | tee -a $LOG_FILE;
}

HES_LOG "READY!";
echo "READY!";

## https://unix.stackexchange.com/questions/144029/command-to-determine-ports-of-a-device-like-dev-ttyusb0
## https://unix.stackexchange.com/questions/132480/case-insensitive-substring-search-in-a-shell-script

cd "$SIGN_WD";

## Display initial search for Arduino message
#echo $SEARCH_ARDUINO_MSG | $SIGN_DEBUG_OUT_BIN $SIGN_CONFIG;
HES_LOG "$MSG_ARDUINO_SCAN";

## Remove case-sensitivity when looking for "Arduino" in USB ID_SERIAL
shopt -s nocasematch;

while [ -z "$LAST_FOUND_DUINO" ]
do
## Scan for plugged Arduino in USB
  for sysdevpath in $(find /sys/bus/usb/devices/usb*/ -name dev); do

          syspath="${sysdevpath%/dev}"
          devname="$(udevadm info -q name -p $syspath)"
          [[ "$devname" == "bus/"* ]] && continue
          eval "$(udevadm info -q property --export -p $syspath)"
          [[ -z "$ID_SERIAL" ]] && continue
          [[ ! "$ID_SERIAL" =~ "Arduino" ]] && continue
          HES_LOG "/dev/$devname - $ID_SERIAL"
          export LAST_FOUND_DUINO="/dev/$devname"

  done
#  echo $MSG_ARDUINO_NONE | $SIGN_DEBUG_OUT_BIN $SIGN_CONFIG;
#  HES_LOG "$MSG_ARDUINO_NONE";
done

#echo "$LAST_FOUND_DUINO";

#echo $MSG_ARDUINO_CONNECT | $SIGN_DEBUG_OUT_BIN $SIGN_CONFIG;
HES_LOG "$MSG_ARDUINO_CONNECT";
HES_LOG "$LAST_FOUND_DUINO";

## Set up the TTY for the Arduino
stty -F $LAST_FOUND_DUINO cs8 $ARDUINO_SERIAL_RATE ignbrk $ARDUINO_STTY_FLAGS;


#echo $MSG_ARDUINO_WAIT | $SIGN_DEBUG_OUT_BIN $SIGN_CONFIG;
HES_LOG "$MSG_ARDUINO_WAIT";

## Wait untill the ready signal comes from the Arduino
#tail -f -n0 $LAST_FOUND_DUINO | grep -qe "$READY_STRING_ARDUINO";

## If tail ended without matching the ready signal from the Arduino, bail
#if [ $? == 1 ]; then
#    #echo $MSG_ARDUINO_READY_FAIL | $SIGN_DEBUG_OUT_BIN $SIGN_CONFIG;
#    HES_LOG "$MSG_ARDUINO_READY_FAIL";
#    exit;
#fi

#echo "$READY_STRING_BBONE" > $LAST_FOUND_DUINO;

# DEBUG
#tail -f $LAST_FOUND_DUINO | egrep --line-buffered 'WARN|ERROR' | tee | $SIGN_DEBUG_OUT_BIN $SIGN_CONFIG;
#tail -f $LAST_FOUND_DUINO;
#cat $LAST_FOUND_DUINO

## Dump buffered input to the sign script
#tail -f $LAST_FOUND_DUINO | egrep --line-buffered 'WARN|ERROR' | $SIGN_BIN_MAIN $SIGN_CONFIG;
cat $LAST_FOUND_DUINO | $SIGN_BIN_MAIN $SIGN_CONFIG >> $LOG_FILE 2>&1;
