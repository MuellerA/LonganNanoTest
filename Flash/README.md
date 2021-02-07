# Flash

Connect the Longan Nano to a W25Q64JV 3V 64MBit SPI Flash Memory (standard mode, no dual or quad I/O)

Read some registers and read / write / erase flash.

# Pin Mapping
| Longan Nano | W25Q64JV |
| ---         | ---      |
| 3V3         | Vcc      |
| B8          | CS       |
| B13 SCK1    | CLK      |
| B14 MISO1   | DO       |
| B15 MOSI    | DI       |
| GND         | GND      |

Access is working but the minimum 4k sector erase size is big compared to Longan's total of 32k RAM when just modifying a few bytes.
