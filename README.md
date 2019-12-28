# raspi-CC2541-programmer

Raspberry Pi GPIO programmer for Texas Instruments CC2541 chip. With this program it is easily possible to flash firmware CC2541 chips. It is possible to flash self developed or manufacturer firmware without need of a CC-debugger device. 

Details about the pinout and debug interface can be found in the official documentation.

With minor code modifications it should be easily possible to use other chips using the same Debug Interface. Essentially it is about different flash memory size.

The chip is used on HM-10 bluetooth modules and its derivates. Since many copies of the original have buggy or not well documented firmware its convenient to flash working and documented firmware like the CC2541hm10v540.bin file located in the repository. 

Firmware files must be 256KiB (262,144B) in size.

## Instructions:

	$ sudo apt install raspberrypi-kernel-headers
	$ git clone https://github.com/exotulip/raspi-CC2541-programmer
	$ cd raspi-CC2541-programmer
	$ ./compile.sh

The desired GPIO lines must be specified in the "pins.cfg" file.
There are three lines to specify. The numbers can be changed as desired. Keep in mind that on the raspberry pi the GPIO line numbers are not the same as the connector header numbers. The

	$ pinout

command helps to clarify.

Example configuration:

	DD:17
	DC:27
	RESET_N:4

Tested on Raspbian Buster and Raspberry Pi 3B. Since Linux is not a real time OS it is recommended to use this program on an idle system in command line mode or over ssh to minimize latency. 
By setting capability as in "compile.sh" the program is allowed to set its own priority to maximum:

	$ sudo setcap 'CAP_SYS_NICE=ep' "./CC2541-programmer"

Available options are:

	--erase-chip
	--dump-flash output.bin
	--flash-firmware input.bin
	--read-id
	--check-debug-lock
	--set-debug-lock

## Example use:


Reading chip ID:

	$ ./CC2541-programmer --read-id

Flashing HM-10 module:

	$ ./CC2541-programmer --flash-firmware CC2541hm10v540.bin

Dump flash if debug lock not set:

	$ ./CC2541-programmer --dump-flash output-file.bin

Checking if files are identical:

	$ diff CC2541hm10v540.bin output-file.bin

## TODO:

There are still some improvements to be done but in general it should work fine.
