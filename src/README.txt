This repository contains the source code of the Blink Detector escape room game. See the Design Documentation for details.

main.c runs on the STM32 and executes the central logic of the game.
ece198.c contains key functions for interfacing with the STM32.
ece198.h is the header file for ece198.c.
blinkDetector.py runs on the Jetson Nano and executes the machine learning operations that determine whether or not a blink has occured. 
