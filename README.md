# BU EC535 Project - Smart Gardening System (SGS)
Abdulaziz AlMailam & Samuel Gulinello

## Abstract

This project was intended to bring together as many topics from EC535 as well as solve a real world problem. Bringing together kernel modules, I2C communication, user space applications, and a QT interface, we developed a system to monitor and provide care to an indoor plant growing system. The product is designed to be autonomous while providing the user with real time metrics displayed on an LCD. By the end of the project we had successfully brought together the different hardware and software components to create a working proof of concept. We successfully processed and presented the input data and simulated the output device functionality.


## Installation
This installation method is for BU ENG affiliates with access to the ENG Grid and assumes basic knowledge of the Linux command line.

1. Download all the given files and verify your current directory
2. Compile the gpio.c kernel module in the /km directory using the given Makefile
4. Run *qmake* with the necessary target, etc.
5. Run *make* with your newly created Makefile from step 3
6. Move your newly created executable called "ec535project" onto the BeagleBone Black, as well as the given shell scripts (pumpToggle.sh, LEDToggle.sh, and ...), and finally your compiled kernel module from step 2.
7. (on the BB) Assuming the sensors are already connected and all the transferred files are in the same directory, first run the shell script to install the GPIO kernel module, then run the main executable compiled in step 5!
