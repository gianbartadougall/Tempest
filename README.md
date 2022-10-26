## Tempest

The major .h files for Tempest are

- version_config.h: Outlines the version of Tempest that will be compiled
- hardware_config.h: Outlines the macros used for configuring all the gpio pins

The major .c files for Tempest are
- hardware_config.c: Configures all the hardware based on the macros defined in hardware_config.h
- tempest.c: Contains the high level logic that runs the system

This project operates as follows

1) Begins in main
2) Initilises tempest.c
    a) Initialises the hardware for all the pins
    b) Initialises abstraction layers for some of the peripherals
3) Enters infinite loop where main system logic is carried out

A brief overview of how the system logic works and each peripheral works is as follows

tempest.c: Infinite while loop where all the flags for each peripheral are checked and appropriate functions are called if the flags are found to be set


Improvements
- Add functionality for the min and max points to be saved on the blind so in the event of a power outage, the blind does not need to be reset as it will configure itself on startup
- Create a custom mcu clock configuration file 


Software to run automated blinds

Features to add
- If you double click into reset mode, you can't get out unless you update the min and max points Make it so you only have to update min and max points if you move the blind outside it's current min max and don't set anything
- Current bug where the blind doesn't go the exact same position every time it goes up and down. It has some drift


Things to do
- Do a review of the code and think about how to structure the code better