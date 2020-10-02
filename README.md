# MPCNC_Joystick
A simple program for controlling my MPCNC by using a Joystick/buttons connected to a second Arduino.
I had [this Joystick module](https://www.google.com/search?safe=active&channel=fs&sxsrf=ALeKk01vf7am_4LdB9LMmfPR0lXqPkCMQQ:1601278914214&source=univ&tbm=isch&q=joystick+shield+v1.a&client=ubuntu&sa=X&ved=2ahUKEwj2uPjmrIvsAhVHiIsKHRgPCRsQjJkEegQICRAB&biw=1920&bih=894) plus an Arduino Uno laying around, so I used the RX/TX on the UNO to send gcode to my MPCNC which runs the modified version of Marlin 1.1.5 (Arduino mega 2560 + Ramps 1.4) that you'll find in this repo.

I thought this would be a piece of cake, but as always things didn't work as expected from the beguining...

#### Buggs I had to find/overcome: 
* The Mega 2560 did not receive anything send on the RX0-serial port (TX works, but not RX?) This made the use of AUX1 on the Ramps useless...
  *  I tried with an USB-RS232 dongle @baud 250000 and could not get the gcode "M117 Test" to be displayed on the LCD, but got the TX-output to be displayed (=correct baud)
    * Setting up serial in ubuntu to baud 250000 was not as easy as it should:
      * $ setserial -a /dev/ttyUSB2 spd_cust
      * $ setserial -a /dev/ttyUSB2 divisor 96
      * $ sudo putty 
        * Session: /dev/ttyUSB0, Speed: 38400, Serial
        * Terminal/Implicit CR in every LF
        * Connection/Serial: /dev/ttyUSB0, 38400, 8, 1, none, XON/XOFF
  * Changed Marlin code to use Serial 1 instead of Serial 0 and removed Zmin/Zmax endstops pin assignment to pins 18/19 (RX1/TX1)
    * This makes sending commands over USB impossible since it's using Serial 0 (D0, D1)
    * Still nothing working when sending commands from the joystick UNO to Marlin...
    * Success in sending gcode "M117 Test" from USB-RS232 to RX1 on Marlin (2 days later... should've tried this earlier)
  * Why why why isn't Marlin working when sending gcode from UNO, but works when sending from USB-adapter?
    * I could confirm sending the correct message from UNO TX when connecting to USB-adapter RX
    * Finally grabbed the multimeter and found out that the TX on the joystick shield that I was using had an voltage devider connected to it to take it down to something that an bluetooth wanted....... 
    * Connectin straight to the TX-pin on the shield was the key to get it working!

#### Usage:
* Upload sketch to UNO
* Connect the joystick shield RX/TX (straight above the UNO RX/TX) to the Ramps Zmin/Zmax [D18/D19](https://m.media-amazon.com/images/S/aplus-media/sc/4dedd672-6684-42a1-88e2-8fe3860f3563.__CR0,0,970,600_PT0_SX970_V1___.jpg) (at least the UNO TX and the Ramps D19) and the 5V/GND to Ramps 5V/GND (just underneath the zmin/zmax)

#### Here's a list of what I have programed it to do: 
* Joystick move = X and Y-moves
* Joystick btn (click down) changes move distances (0.1, 1, 10, 100mm)
  * Long press => changes between normal mode (up down btns = Z) and XY-only-mode (makes buttons up/down controll Y instead of Z)
* Buttons:
  * Upp (A)  => Z+ (or Y+ if in XY-mode)
  * Down (C) => Z- (or Y- if in XY-mode)
  * Left (D) => X-
  * Right (B) => X+
  * Left + Right (keep down 500ms+) => Reset coordinates to this location (X0 Y0 Z0)
  * Button E => Home z (make sure you have touch-plate!!!)
  * Button F => Home X and Y (endstops)

If you are using my modified version of Marlin, you'll see that the LCD-screen will show you what mode you are in and how far you'll travel on each command. 

/Kalle
