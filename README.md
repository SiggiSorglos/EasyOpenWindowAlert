# EasyOpenWindowAlert
energy saving device: sounding alarm when a window is open for more than 5 minutes.

This project contains the code and the schematic for a device to save energy.
A lot of energy is wasted by left open windows during the winter. 
This can be avoided by alarming the user that the window is still open.

A simple device will sound an alarm if a window is open for more than 5 minutes.
The open window is detected with a reed switch.
The reed switch is toggled by a magnet mounted to the window.

steps to get started:
1. program an ATtiny85 in the Arduino IDE with the code
2. build the simple electronic around it (see schematic) and plug in the ATtiny85 to the socket.
3. Optional: build a housing i. e. with a 3D printer (see openscad files)
4. Mount the device next to a window and the magnet on the window. 

Theory of operation:
As long as the window is closed, the reed switch is closed and the ATtiny85 sleeps. Therefore the current consumption is very low.
The circuit should run on 2 AAA Batteries for > 2 years.
When the window is opened the switch opens. The LED lights up for 2 seconds to indicate that the "window opened" event was detected.
To indicate that the timer is running, the LED flashes every 4 seconds.
If the window is open for 5 Minutes, a sound (three beeps) will alarm the user to close the window.
If the window is not closed, the alarm will repeated after 5 Minutes.
If the window is closed, or the alarm was repeated three times, the ATtiny85 will go to sleep again.
