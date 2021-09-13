# Measurement-of-Distance
Design a distance meter: An IR distance sensor converts distance into 
voltage. The software will use the 12-bit ADC built into the microcontroller. The ADC will be 
sampled at 20 Hz using SysTick interrupts. Write a C function that converts the ADC 
sample into distance, with units of 1 cm. That data stream will be passed from the ISR into 
the main program using a mailbox, and the main program will output the data on an LCD 
display.
