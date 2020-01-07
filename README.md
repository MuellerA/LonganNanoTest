<h1>Tests</h1>

<table>
  <tr><th>Led</th><td>Onboard RGB Led</td></tr>
  <tr><th>Freq</th><td>Output wave signal on pin A8/CK_OUT0 (select wave form in main())</td></tr>
  <tr><th>Usart</th><td>USART0 (115200,8N1), Pins TX0, RX0</td></tr>
  <tr><th>Lcd</th><td>Onboard LCD 160x80, SPI master</td></tr>
  <tr><th>Dma</th><td>DMA mem to CRC register</td></tr>
  <tr><th>UsartIrq</th><td>USART0 (115200,8N1), Echo serial input to serial output and LCD with 10 character per second, using 1k buffer</td></tr>
  <tr><th>PA8</th><td>Read GPIO PA8 which is connected to BOOT0 button</td></tr>
  <tr><th>EspLink</th><td>Connect Rx0/Tx0/Gnd/Rst to an ESP8266 running <a href="https://github.com/jeelabs/esp-link/releases/tag/v2.2.3">esp-link v2.2.3</a></td></tr>
  <tr><th>EspLinkWeb</th><td>Connect Rx0/Tx0/Gnd/Rst to an ESP8266 running <a href="https://github.com/jeelabs/esp-link/releases/tag/v3.2.47.alpha">esp-link v3.2.47</a> and update a User Defined Web Page (copy from html folder)</td></tr>
  <tr><th>I2c</th><td>Connect a 3.3V 1602 LCD via PCF8574 (I2C-to-parallel) to port I2C0 (pins B6,B7) in master mode</hd></tr>
</table>

<h1>Resources</h1>

<img src="https://longan.sipeed.com/assets/longan_nano_pin_map.png" alt="pin map" width="100%"/>

<h2>Boards</h2>
<p>
<table>
<tr><th>Board</th><th>MCU</th><th>Flash</th><th>SRAM</th></tr>
<tr><td>Longan Nano</td><td>GD32VF103CBT6</td><td>128kB</td><td>32kB</td></tr>
<tr><td>Longan Nano Lite</td><td>GD32VF103C8T6</td><td>64kB</td><td>20kB</td></tr>
</table>
</p>

<h2>Documentation, Examples, etc</h2>
<table>
  <tr><td>Sipeed Longan Nano <a href="http://dl.sipeed.com/LONGAN/Nano/">Specs Tools</a> <a href="https://longan.sipeed.com/en/">Wiki</a> <a href="https://bbs.sipeed.com/c/14-category">BBS</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/222/path_type/1">GD32VF103 User Manual 1.2</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/221/path_type/1">GD32VF103 Datasheet 1.1</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/228/path_type/1">GD32VF103 Firmware Library User Guide 1.0</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/227/path_type/1">GD32VF103 Demo Suites 1.0.2 (.rar)</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/223/path_type/1">GD32VF103 Firmware Library 1.0.1 (.rar)</a></td></tr>
  <tr><td><a href="https://content.riscv.org/wp-content/uploads/2019/06/riscv-spec.pdf">The RISC-V Instruction Set Manual Volume I: Unprivileged ISA</a></td></tr>
</table>

<h2>platformio.ini</h2>
<pre>
[env:sipeed-longan-nano-lite]
platform = gd32v
board = sipeed-longan-nano-lite
framework = gd32vf103-sdk
upload_protocol = dfu
&nbsp;
[env:sipeed-longan-nano]
platform = gd32v
board = sipeed-longan-nano
framework = gd32vf103-sdk
upload_protocol = dfu</pre>

Longan Nano Lite needs the <a href="https://github.com/sipeed/platform-gd32v">development version of the GD32V platform</a> (2019-12).

<h2>DFU Upload (Linux)</h2>
<ol>
 <li>connect USB</li>
 <li>press buttons RESET, BOOT0</li>
 <li>release button RESET</li>
 <li>release button BOOT0</li>
 <li>start upload in PlatformIO</li>
</ol>

<h2>PIN Connections</h2>
<table>
  <tr><th colspan="3">A</th><th colspan="3">B</th><th colspan="3">C</th><th colspan="3">D</th>
  <tr><th>Pin</th><th>5VT</th><th>OnBoard Function</th><th>Pin</th><th>5VT</th><th>OnBoard Function</th><th>Pin</th><th>5VT</th><th>OnBoard Function</th><th>Pin</th><th>5VT</th><th>OnBoard Function</th></tr>
  <tr><td>A0</td><td>-</td><td></td>                    <td>B0</td><td>-</td><td>LCD RS</td>                             <td></td><td></td><td></td>  <td>[D0/OSCIN]</td><td>-</td><td>8MHz Clk</td></tr>
  <tr><td>A1</td><td>-</td><td>Green LED active low</td><td>B1</td><td>-</td><td>LCD RST</td>                            <td></td><td></td><td></td>  <td>[D1/OSCOUT]</td><td>-</td><td>8MHz Clk</td></tr>
  <tr><td>A2</td><td>-</td><td>Blue LED active low</td> <td>[B2]</td><td>*</td><td>LCD CS</td></tr>
  <tr><td>A3</td><td>-</td><td></td>                    <td>B3</td><td>*</td><td></td></tr>
  <tr><td>A4</td><td>-</td><td></td>                    <td>B4</td><td>*</td><td></td></tr>
  <tr><td>A5/SCK0</td><td>-</td><td>LCD SPI Clk</td>    <td>B5</td><td>-</td><td></td></tr>
  <tr><td>A6</td><td>-</td><td></td>                    <td>B6</td><td>*</td><td></td></tr>
  <tr><td>A7/MOSI0</td><td>-</td><td>LCD SPI MOSI</td>  <td>B7</td><td>*</td><td></td></tr>
  <tr><td>A8</td><td>*</td><td>10k PullDown?, connected to BOOT0 button active high</td><td>B8</td><td>*</td><td></td></tr>
  <tr><td>A9</td><td>*</td><td></td>                    <td>B9</td><td>*</td><td></td></tr>
  <tr><td>A10</td><td>*</td><td></td>                   <td>B10</td><td>*</td><td></td></tr>
  <tr><td>A11/USBFS_DM</td><td>*</td><td>USB D-</td>    <td>B11</td><td>*</td><td></td></tr>
  <tr><td>A12/USBFS_DP</td><td>*</td><td>USB D+</td>    <td>B12/NSS1</td><td>*</td><td>TF CARD SPI CS</td>               <td>C13</td><td>-</td><td>Red LED active low</td></tr>
  <tr><td>A13</td><td>*</td><td></td>                   <td>B13/SCK1</td><td>*</td><td>TF CARD SPI Clk</td>              <td>C14/OSC32IN</td><td>-</td><td>32kHz Clk</td></tr>
  <tr><td>A14</td><td>*</td><td></td>                   <td>B14/MISO1</td><td>*</td><td>TF CARD SPI MISO, 10k PullUp</td><td>C15/OSC32OUT</td><td>-</td><td>32kHz Clk</td></tr>
  <tr><td>A15</td><td>*</td><td></td>                   <td>B15/MOSI1</td><td>*</td><td>TF CARD SPI MOSI, 10k PullUp</td></tr>
  <tr><td colspan="99">[x] no available as header pin; 5VT (5 Volt Tolerant Input): * yes, - no</td></tr>
</table>
