# MPCNC_Joystick
A simple program for controlling my MPCNC by using a Joystick/buttons connected to a second Arduino.
I had [this Joystick module](https://www.google.com/search?safe=active&channel=fs&sxsrf=ALeKk01vf7am_4LdB9LMmfPR0lXqPkCMQQ:1601278914214&source=univ&tbm=isch&q=joystick+shield+v1.a&client=ubuntu&sa=X&ved=2ahUKEwj2uPjmrIvsAhVHiIsKHRgPCRsQjJkEegQICRAB&biw=1920&bih=894) plus an Arduino Uno laying around, so I used the RX/TX on the UNO to send gcode to my MPCNC which runs the [modified version of Marlin 2.0.x](https://github.com/klalle/Marlin/tree/V1CNC_Ramps_Dual_Kalle)(Arduino mega 2560 + Ramps 1.4) - This joystick-arduino code should work with any Marlin version (tested on 1.1.5 as well), and probably on different HW setups as well (Rambo etc) as long as you connect it to a working serial connection.
The code doesn't just spam Marlin with gcode commands (as I have seen other implementations do...), before each new command it waits for an "ok" from Marlin and tries to calculate the delay to as long as the move should take (not considering acceleration...).


#### Usage:
* Upload sketch to UNO
* Connect the joystick shield RX/TX (straight above the UNO RX/TX) to the Ramps Zmin/Zmax [D18/D19](https://m.media-amazon.com/images/S/aplus-media/sc/4dedd672-6684-42a1-88e2-8fe3860f3563.__CR0,0,970,600_PT0_SX970_V1___.jpg) and the 5V/GND to Ramps 5V/GND (just underneath the zmin/zmax)

#### Here's a list of what I have programed it to do: 
**"longclick"** defenition: keep btn pressed +1.5s

* Joystick btn (longclick) => turns the joystick on/off (starts listening for commands) - overridden if btn connected to pin D9
* Joystick move => X and Y-moves 
  * first move has a 300ms delay to enable single move, if kept activated a continuos move is started till released. 
* Buttons:
  * Upp (A)  => Z+
  * Down (C) => Z-
  * Left (D) (longclick) => Home XY
  * Right (B) (longclick) => Set home here (resets Marlin to X0 Y0 Z0)
  * E (extra-longclick) => Home z (make sure you have touch-plate connected and configured on some available pin)
  * F => Changes single move distance (0.1, 1, 10, 100mm/20mm) (changes the led's)


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
  * Changed Marlin code to use Serial 1 as well as Serial 0 and removed Zmin/Zmax endstops pin assignment to pins 18/19 (RX1/TX1)
    * Still nothing working when sending commands from the joystick UNO to Marlin...
    * Success in sending gcode "M117 Test" from USB-RS232 to RX1 on Marlin (2 days later... should've tried this earlier)
  * Why why why isn't Marlin working when sending gcode from UNO, but works when sending from USB-adapter?
    * I could confirm sending the correct message from UNO TX when connecting to USB-adapter RX
    * Finally grabbed the multimeter and found out that the TX on the joystick shield that I was using had an voltage devider connected to it to take it down to something that an bluetooth wanted....... 
    * Connectin straight to the TX-pin on the shield was the key to get it working!

If you are using my [modified version of Marlin](https://github.com/klalle/Marlin/tree/V1CNC_Ramps_Dual_Kalle), you'll see that the LCD-screen will show you what mode you are in and how far you'll travel on each command. 

/Kalle
