BBBFreeRTOS
===========

FreeRTOS for BeagleBone Black
source:https://github.com/henfos/BBBFreeRTOS

----------------------
Makefile located in in Demo/AM3359_BeagleBone_GCC/
command:make && make app

copy the MLO(made from ti StarterWare) and app into microsd card 
push S2 button and turn on beaglebone black

Working:
- System tick
- Interrupts
- GPIO
- Output/Input serial
- shell
- mmu

Not finished:
- An ISR routine for serial input //Should however be no problem as core FreeRTOS is fully functioning 

-----------------------
Test main.c file
use help command in shell ,it will show the function.
