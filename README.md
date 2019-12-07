<img src="https://longan.sipeed.com/assets/longan_nano_pin_map.png" alt="pin map"/>

<p>
<table>
<tr><th>Board</th><th>MCU</th><th>Flash</th><th>SRAM</th></tr>
<tr><td>Longan Nano</td><td>GD32VF103CBT6</td><td>128kB</td><td>32kB</td></tr>
<tr><td>Longan Nano Lite</td><td>GD32VF103C8T6</td><td>64kB</td><td>20kB</td></tr>
</table>
</p>

<table>
  <tr><th colspan="99">Documentation, Examples, etc</th></tr>
  <tr><td>Sipeed Longan Nano <a href="http://dl.sipeed.com/LONGAN/Nano/">Specs Tools</a> <a href="https://longan.sipeed.com/en/">Wiki</a> <a href="https://bbs.sipeed.com/c/14-category">BBS</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/222/path_type/1">GD32VF103 User Manual 1.2</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/221/path_type/1">GD32VF103 Datasheet 1.1</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/228/path_type/1">GD32VF103 Firmware Library User Guide 1.0</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/227/path_type/1">GD32VF103 Demo Suites 1.0.2 (.rar)</a></td></tr>
  <tr><td><a href="http://gd32mcu.21ic.com/en/down/document_id/223/path_type/1">GD32VF103 Firmware Library 1.0.1 (.rar)</a></td></tr>
  <tr><td><a href="https://content.riscv.org/wp-content/uploads/2019/06/riscv-spec.pdf">The RISC-V Instruction Set Manual Volume I: Unprivileged ISA</a></td></tr>
</table>

<p>platformio.ini</p>
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

<p>DFU Upload (Linux)
<ol>
 <li>connect USB</li>
 <li>press buttons RESET, BOOT0</li>
 <li>release button RESET</li>
 <li>release button BOOT0</li>
 <li>start upload in PlatformIO</li>
</ol>
</p>

<table>
  <tr><th colspan="99">PINs</th></tr>
  <tr><td>C13</td><td>Red LED</td></tr>
  <tr><td>A1</td><td>Green LED</td></tr>
  <tr><td>A2</td><td>Blue LED</td></tr>
  <tr><td>A5/SCK0</td><td>LCD SPI Clk</td></tr>
  <tr><td>A7/MOSI0</td><td>LCD SPI MOSI</td></tr>
  <tr><td>B0</td><td>LCD RS</td></tr>
  <tr><td>B1</td><td>LCD RST</td></tr>
  <tr><td>B2</td><td>LCD CS</td></tr>
</table>

<table>
  <tr><th colspan="99">Tests</th></tr>
  <tr><th>Led</th><td>Onboard RGB Led</td></tr>
  <tr><th>Freq</th><td>Output wave signal on pin A8/CK_OUT0 (select wave form in main())</td></tr>
  <tr><th>Usart</th><td>USART0 (115200,8N1), Pins TX0, RX0</td></tr>
</table>
