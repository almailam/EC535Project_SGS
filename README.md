# BU EC535 Project - Smart Gardening System (SGS)
Abdulaziz AlMailam & Samuel Gulinello


Installation method for BU ENG students on the ENG Grid:

1. Download all the given files and verify your current directory
2. Compile the gpio.c kernel module in the /km directory using the given Makefile
4. Run *qmake* with the necessary target, etc.
5. Run *make* with your newly created Makefile from step 3
6. Move your newly created executable called "ec535project" onto the BeagleBone Black, as well as the given shell scripts (pumpToggle.sh, LEDToggle.sh, and ...), and finally your compiled kernel module from step 2.
7. (on the BB) Assuming the sensors are already connected and all the transferred files are in the same directory, first run the shell script to install the GPIO kernel module, then run the main executable compiled in step 5!
