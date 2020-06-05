# Adjustable Constant Load, Constant Current and Constant Power

Based on the work of GreatScott https://www.instructables.com/id/DIY-Adjustable-Constant-Load-Current-Power/

## Improvements

* better handling of the rotary encoder
* the original project was a little bit dangerous. If the Arduino boots or resets during probing, the mosfet was fully open for about 2 seconds. A 10k resistor solved this
* the glas fuse was replaced by a polyfuse, 3A
* added temperature control using a 1wire DS18B20 sensor. Includes shutdown of the whole device if temperature alarm was triggered
* added software based over current protection in source code
* switched to INA219 sensor for measuring voltage and current, makes voltage divider and ACS712 obsolete. This sensor can handle up to 26V and 3.2A
* Added a 3d printed case to the repository. 

## BOM, tools used by me

* Arduino Nano
* INA219 breakout
* MicroUSB breakout (I am powering my 5V benchstuff using MicroUSB, this is not needed if you supply 5V in another way)
* Mosfet IRFZ44N
* Mosfet driver IC TC4420, maybe 2x4 IC socket as well?
* some 2.54mm pinheaders
* some wires
* Resistors, 0.25W: 10R, 4k7, 10k
* Electrolytic capacitors, 10V: 22uF, 470uF
* Polyfuse 3A
* LCD 16x2, I2C
* Rotary Encoder
* Dupont cables female female
* 2 binding posts 4mm
* 1 switch SPST
* 1 heatsink, big enough to deal with the heat
* 5V power supply
* M4 screws to mount temperatur sensor and mosfet to the heatsink
* Solder stuff
* some basic tools like a screw driver, alan keys and so on
* M3.5 drill and M4 tap and die tool to add holes into the heatsink of not there already
* M4.5 drill to widen the hole in the TO220 case of the mosfet.

if you are using the 3D printed case I am providing in subfolder stl

* M3 screws to mount the housing parts to each other
* M2 screws to fix the LCD in its place
* a 3D printer and some filament of your choice
 
## License: 

https://creativecommons.org/licenses/by-nc-sa/4.0/

Most obvious limits of this license:
* You are not allowed to use this stuff for commercial purposes
* You can download, reupload and change the stuff, but have to release it under the same license again
* You have the mention the origin, "Original by Peter Stimpel, https://github.com/pstimpel/caterpillarbulldozer/"

## Home of this project

https://github.com/pstimpel/ConstantLoad



![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/0.jpg)
 
![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/1.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/2.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/3.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/4.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/5.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/6.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/7.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/8.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/9.jpg)

![IMAGE ALT TEXT HERE](https://github.com/pstimpel/ConstantLoad/raw/master/media/a.jpg)

## Build instructions

Just follow the guide by GreatScott. Different steps are:

1.) Use the circuit provided by me to take advantage of temperature control, some more security and the different sensor

2.) Use the Arduino sketch provided by me to take advantage of temperature control, some more security and the different sensor. On top you will find the use of this device a little bit easier. At least I do.

## 3D print instructions

There is nothing special. I used PLA, no supports needed. 3 shells and 10% infill did it. 

## Feedback

If you go for a build, I would like it very much to hear from you. Maybe you want to share pics of your Bulldozer? You can find me at

https://twitter.com/PjotrS72

https://www.prusaprinters.org/social/2411-pstimpel/prints

https://www.thingiverse.com/pstimpel

## Finally...

I am providing this stuff as it is. I do not promise anything, and I am not responsible for any harm you might create or receive by following this manual or by using the methods and results described in this manual.

### Warning

High current / power can cause heat, and heat can cause injuries. Make sure you adapt the warnlevels on top of the Arduino sketch to your needs. Especially the temperature alarm threshold should be set below the melting temperature of your filament if you were going for the 3D printed case.