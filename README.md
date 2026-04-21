# Final Project: Commanding Robot through USART


actual tested HC-05 UART baud = 9600
do not trust the uploaded datasheet for this module variant

You run bind when you need to create the /dev/rfcomm1 mapping
after that, opening /dev/rfcomm1 should trigger the connection
you only need to run bind again if the binding is gone, released, or lost after reboot/service restart

sudo rfcomm bind 1 00:14:03:05:09:9E 1

Fixing character device
sudo rfcomm release 1 2>/dev/null
sudo rm -f /dev/rfcomm1
sudo rfcomm bind 1 00:14:03:05:09:9E 1
ls -l /dev/rfcomm1