# Bluetooth_chat
this is an application developed for chatting between two devices , one is the linux operating system (tested on ubuntu ) and an android mobile (version 4.4 or above).
One needs to run this coonect.c file in your terminal and at the same time you need to open one application in mobile named 
withoutpair.
You can download the .apk file for withoutpair from the repository .
make sure to turn on bluetooth on both devices and make the mobile bluetooth discoverable .
now once it is connected you can happily chat between a laptop and a mobile without any internet connection.

to run the .c file on ubuntu run the following command :

gcc connect.c -lbluetooth
