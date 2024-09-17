#!/bin/bash

unexport_gpio() {
	
	# Unexport Pins
	echo 16 > /sys/class/gpio/unexport
	echo 17 > /sys/class/gpio/unexport
	echo 21 > /sys/class/gpio/unexport
	echo 23 > /sys/class/gpio/unexport
	echo 47 > /sys/class/gpio/unexport
	
	exit 0
}

# Intercept CTRL+C
trap unexport_gpio INT

# Export Pins to make it accessible
echo 16 > /sys/class/gpio/export
echo 17 > /sys/class/gpio/export
echo 21 > /sys/class/gpio/export
echo 23 > /sys/class/gpio/export
echo 47 > /sys/class/gpio/export

# Configure the GPIOs in input
echo in > /sys/class/gpio/pioA16/direction
echo in > /sys/class/gpio/pioA17/direction
echo in > /sys/class/gpio/pioA21/direction
echo in > /sys/class/gpio/pioA23/direction
echo in > /sys/class/gpio/pioB15/direction

while [ 1 ]; do
	
	printf "\033c"
	printf "JoyStick :\n\n"
	
	printf "  "
	
	# Left
    if [ $(cat /sys/class/gpio/pioA16/value) -eq 0 ]; then
        printf "←"
    fi

	# Up
    if [ $(cat /sys/class/gpio/pioA21/value) -eq 0 ]; then
        printf "↑"
    fi

	# Down
    if [ $(cat /sys/class/gpio/pioA23/value) -eq 0 ]; then
        printf "↓"
    fi

	# Right
    if [ $(cat /sys/class/gpio/pioA17/value) -eq 0 ]; then
        printf "→"
    fi
	
	# Button
	if [ $(cat /sys/class/gpio/pioB15/value) -eq 0 ]; then
        printf "■"
    fi
	
	printf "\n\n"
	
	sleep 0.1
	
done