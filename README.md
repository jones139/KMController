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
The serial interface to the microcontroller needs to be /dev/kmirror.
This can be achieved with a udev rule such as /etc/udev/rules.d/92-kmirror.rules which contains the following:

```# Force K-mirror arduino nano board to by symlinked to /dev/kmirror
# idVendor=1a86, idProduct=7523
ACTION=="add", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", SYMLINK+="kmirror"
```
The idVendor and idProduct need to match the particular device in use (use lsusb or dmesg)

Compile and upload to the microcontroller with:
```
make
make upload
```

you can monitor the output from the microcontroller with
```
make monitor
```

To exit the monitor screen do ctrl-a ctrl-d.   This detatches the terminal from the monitor but leaves it running reattach with screen -r or kill the process.
