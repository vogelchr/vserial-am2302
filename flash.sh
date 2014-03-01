#!/bin/sh

if ! [ -f "vserial-app.hex" ] ; then
	echo "Please run make first to create vserial-app.hex." >&2
	exit 1
fi

avrdude -p atmega32u4 -c dragon_isp -P usb -U flash:w:vserial-app.hex
