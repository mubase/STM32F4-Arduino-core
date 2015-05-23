# STM32F4-core-for-STM32-Arduino 

This is a clone of the STM32F4 core for Roger Clark's STM32 Arduino project hosted here:
https://github.com/rogerclarkmelbourne/Arduino_STM32

The repository includes changes to spi.c, spi.h, rccF2.h and rccF2.c in order for the STM32F4 discovery's CS43l22 audio codec to be used in 16 bit 48KHz Phillips standard mode.

A new file has been added which includes two main functions:

codec_16_bit_setup();    // This sets up I2S3 on port C and I2C on port B to initialise the codec. The function is put in setup().

codec_send_data() sends 16 bit data over I2S to the codec and audio is sent to the headphone socket.

An example is provided.

I will eventually get around to making a proper library for I2S and the codec with all set up commands selectable by the user.... but for now we've got 16 bit audio!!!


 
