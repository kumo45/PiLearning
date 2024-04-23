Tool used for creation of schematics: TinyCAD
=======
gcc -o ute blinky_diode.c -lpigpio -pthread - for blinking diode w/ potentiometer dependent frequency
gcc -o ute button.c -lpigpio - for button controlled diode