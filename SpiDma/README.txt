The program calculates an Apfelmännchen (Madelbrot set) 80x80 pixel, 2 byte per pixel and copies it via SPI to the display in a loop.

The program can be run without DMA, with synchronous DMA (start DMA and then wait to finish) and with asynchronous DMA (calculation and copy in parallel), configuration in spi.cpp Spi::copy(). The SPI frequency can be set by the prescaler value (PSC) in lcd.cpp Lcd::setup(). Because of the asynchronous DMA a double buffer is used (one filled while the other is copied).

The display shows the time spent for calculation and copy, the total time and the iteration count.
 
Measurement PSC 256 (slow SPI)
ms      CPU copy      sync DMA     async DMA
calc       192           192           192
copy       242           242             0
total      435           435           242

Measurement PSC 4 (fast SPI)
ms      CPU copy      sync DMA     async DMA
calc       192           192           192
copy         5             3             0
total      197           195           192

Conclusion:
No significant difference between CPU copy and synchronous DMA.
Async DMA is only useful if SPI is slow, but then it needs lots of RAM for the double buffer (2 times 80x80x2 byte = 25.6k of Longan's 32k. Calculating only quarter images would reduce the required RAM but would make the program more complicated.

And now set the prescaler back to 4 and enjoy the Apfelmännchen! ;-)
