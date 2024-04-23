<<<<<<< HEAD
gcc -o ute main.c :P

Tool used for creation of schematics: TinyCAD
=======
gcc -o ute blinky_diode.c -lpigpio -pthread - for blinking diode w/ potentiometer dependent frequency
gcc -o ute button.c -lpigpio - for button controlled diode
>>>>>>> c0556ce (added diode blinking; split blinking and button into separate c files)
