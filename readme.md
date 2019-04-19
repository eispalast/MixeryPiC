MixeryPi C
==========

*A hardware Mixer based on the Raspberry Pi to control the Behringer X-Air 18*

Features
--------

* Control your Behringer X-Air 18 Wire hardware buttons and rotary encoders
* 8 buttons to choose channel
* button to switch layers so you can choose from channel 1-8 or 9-16
* button to mute all channels
* rotary encoder to choose output (Main, 6 AUX-Busses) Push button to toggle fine-mode (control even more precisely)
* rotary encoder to increase/lower a channel's volume. Push button to mute channel
* WiFi and/or ethernet connection possible
* 2x16 LCD 

Required components
-------------------
* Raspberry Pi (I used a 3 B+ )
* 2x16 LCD (HD44780)
* 10 pushbuttons
* 2x rotary encoder (KY-040)
* Resistors:
    * 10x 2k Ohm
    * 4x 5k Ohm
    * 1x 10k Ohm potentiometer
* 4x 100nF capacitors
* Micro USB and ethernet mountable extension cables
* Some kind of housing
* Flat cable
* Screws and spacers
* 2x potentiometer knobs

Tools
-----
* Pliers
* Soldering iron
* Drill
* box cutter
* file

Installation
-----------

* Install Raspbian
* Maybe you have to install [WiringPi](http://wiringpi.com/ "WiringPi official homepage")
* Download all files from this Repository
* Connect to your X-Air 18 via WiFi (or to an external router that your X-Air is connected to)
* Enable SSH on the Pi for future updates and maintenance ([HowTo](https://www.raspberrypi.org/documentation/remote-access/ssh/ "Activate SSH on Raspberry Pi"))
* cd to the path where you saved this repository
* type `make` in the console and press enter
* ignore the warning
* to autostart the program:
    * `cd` to /etc/ and open rc.local : `sudo nano rc.local`
    * add the path to the programm, for example: `home/pi/MixeryPi/mix &`
    * don't forget the *&*-symbol. Click [here](https://www.raspberrypi.org/documentation/linux/usage/rc-local.md "Information about rc.local") for further information.
* Wire up the hardware as shown in this ![Sketch](https://gitlab.tubit.tu-berlin.de/timo.schlegel/MixeryPiC/blob/master/MixeryPi%20C%20Sketch.svg "Fritzing Sketch") and in this table:

| GPIO-Pin | Connected to               |
| -------- | -------------------------- |
| 4        | LCD RS                     |
| 17       | LCD E                      |
| 18       | LCD DATA4                  |
| 22       | LCD DATA5                  |
| 23       | LCD DATA6                  |
| 24       | LCD DATA7                  |
| 26       | Button Channel 1           |
| 21       | Button Channel 2           |
| 20       | Button Channel 3           |
| 16       | Button Channel 4           |
| 12       | Button Channel 5           |
| 7        | Button Channel 6           |
| 8        | Button Channel 7           |
| 25       | Button Channel 8           |
| 19       | Button Toggle Layer        |
| 13       | Button Mute All Channels   |
| 9        | Button Toggle Fine mode    |
| 6        | Button Toggle Mute Channel |
| 5        | Volume knob CLK            |
| 11       | Volume knob DT             |
| 10       | Choose output knob CLK     |
| 27       | Choose output knob DT      |

You can choose different GPIO-pins, but make sure to change them in the `define`-section in the main.c-file.





Future updates
--------------

Yeah, yeah I know: Putting functions in the header-files is kind of weird and not how it is supposed to be. But it works ;) I will change that in the future. There is a lot of refactoring needed.
I also want to add some more features step by step and maybe some day I can control each and every parameter with this gadget.

Next I want to add:
* controlling the FX-Channels 
* controlling each parameter of the EQ for each channel. Maybe I'll get rid of the
fine-mode in the future and make that button some kind of *Shift*-button which triggers different modes
when pressed together with others buttons.