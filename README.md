<img src="https://longan.sipeed.com/assets/longan_nano_pin_map.png" alt="pin map"/>

<p>
<table>
<tr><th>Board</th><th>MCU</th><th>Flash</th><th>SRAM</th></tr>
<tr><td>Longan Nano</td><td>GD32VF103CBT6</td><td>128kB</td><td>32kB</td></tr>
<tr><td>Longan Nano Lite</td><td>GD32VF103C8T6</td><td>64kB</td><td>20kB</td></tr>
</table>
</p>

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
</table>

<table>
  <tr><th colspan="99">Tests</th></tr>
  <tr><th>Led</th><td>Onboard RGB Led</td></tr>
  <tr><th>Freq</th><td>Output wave signal on pin A8/CK_OUT0 (select wave form in main())</td></tr>
</table>
