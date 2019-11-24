KMController
============

KMController isdesigned to control a stepper motor driven rotating mirror
using an arduino (ATMEGA 328) based microcontroller.

It is programmed via a USB serial interface, and accepts the following
commands:

 * HOME : Rotate in the reverse direction until the home limit switch is activated.
 * START : start cycing between the minimum and maximum programmed positions.
 * STOP  : stop moving.
 * MOVETO=p : move to the given position and stop
 * SPEED=p : set the movement speed to <p>
 * ACC=a : set the acceleration to <a>
 * CMAX=p : Set the maximum position during cycling to <p>
 * CMIN=p : Set the minimumposition during cycling to <p>


Installation
------------

make
make upload

....should work....failing that try the arduino IDE.
I'm having trouble with
make monitor
which doesn't echo anything I type and although I can see the comms LED
flashing on the board, nothing appears on the screen, but it works with the
Arduino IDE monitor.
