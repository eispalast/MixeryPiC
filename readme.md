MixeryPi C
==========

*A hardware Mixer based on the Raspberry Pi to control the Behringer X-Air 18*

Features
--------

Installation
-----------

* Install Raspbian
* Download all files from this Repository
* Connect to your X-Air 18 via WiFi
* Enable SSH on the Pi for future updates and maintenance ([HowTo](https://www.raspberrypi.org/documentation/remote-access/ssh/ "Activate SSH on Raspberry Pi"))
* cd to the path where you saved this repository
* type `make` in the console and press enter
* ignore the warning
* to autostart the program:
    * `cd` to /etc/ and open rc.local : `sudo nano rc.local`
    * add the path to the programm, for example: `home/pi/MixeryPi/mix &`
    * don't forget the *&*-symbol. Click [here](https://www.raspberrypi.org/documentation/linux/usage/rc-local.md "Information about rc.local") for further information.
* Wire up the hardware as shown in this ![Sketch](https://gitlab.tubit.tu-berlin.de/timo.schlegel/MixeryPiC/blob/master/MixeryPi%20C%20Sketch.svg "Fritzing Sketch")


