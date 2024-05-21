##Test field to learn how to control pi using c

Tool used for creation of schematics: TinyCAD<br><br>

Build instructions:<br>
gcc -o ute blinky_diode.c -lpigpio -pthread - for blinking diode w/ potentiometer dependent frequency<br>
gcc -o ute button.c -lpigpio - for button controlled diode<br>
gcc -o ute ControllerReader.c  - for displaying inputs from connected controller<br>
gcc -o ute command_queue.c -pthread -lm - for command-line controlled servos<br>
gcc -o ute svc.c -pthread -lm - for controller controlled servos<br>