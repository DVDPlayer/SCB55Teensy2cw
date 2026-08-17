Disclaimer: I'm not a professional programmer or hardware engineer, so go easy on me, use at own risk!

Note: I know that the Teensy 2.0 is quite old, but I used it because I had it and for the challenge!
	Feel free to port it to another device. (Teensy 2.0 also has built-in support for USB-MIDI)

# 🚀 Teensy 2.0 Custom MIDI SBC-55 Controller Firmware (With Hardware Rev.1)

A playback prioritizing serial and USB MIDI firmware for the Teensy 2.0 platform featuring an OLED user interface, rotary encoder control (variable acceleration), Capital Tone Fallback (inverts bank# on fallback), drum and instrument tracking / setting per channel (354 instruments & 10 drum sets - all banks except 126), channel level settings (volume, reverb, chrous), global level settings (volume reverb, chrous), MT-32 emulation initialization (patch, pan, reverb, & pitch bend), display of MT-32 / SC-55 SysEx text messages, display of SC-55 SysEx graphics, send / receive GM & GS SysEx reset, 'analog' volume setting, all notes off Panic! feature, external (software driven) reset button, power-on reset of SCB-55, inactivity screen saver, and a dedicated MIDI hardware pipeline to the SCB-55 waveblaster module.  

Runs from USB (<350mA) or dedicated 6Vdc to 35Vdc input. (see Teensy MOD picture)

This repository contains only the core project firmware and hardware. All third-party dependencies are managed locally to keep the repository structure clean and as lightweight as possible.  

---

## 📦 Required Dependencies

Before compiling, make sure you have the following libraries installed via the Arduino Library Manager:
* **Adafruit_GFX** (OLED Core Graphics)
* **Adafruit_SSD1306** (OLED Driver)
* **Encoder** by Paul Stoffregen (Hardware Rotary Encoder tracking)

*Note: Core libraries (`Arduino.h`, `Wire.h`, `avr/pgmspace.h`, `util/delay.h`) are built into the Teensy / AVR toolchain and do not need to be installed manually.*

---

## 🛠️ Installation & Memory Optimization

To successfully fit the compiled firmware within the microcontroller's limited flash memory, you must disable the default Adafruit splash screen to save code space.

This project is shared under the **GNU GPLv3** license, and since third-party libraries are not hosted directly in this repository, you will need to apply the following optimizations to your local Arduino environment manually:

### Required Tweaks:
1. Locate your local Arduino libraries folder (usually found under `Documents/Arduino/libraries/`).
2. Open the `Adafruit_SSD1306` directory.
3. Open `Adafruit_SSD1306.h` in a text editor.
4. Locate the configuration lines near the top of the file and **uncomment** the splash logo macro definition to match this:
```   
   // Uncomment to disable Adafruit splash logo
   #define SSD1306_NO_SPLASH
```   
5. Open the Arduino `usb_api.h` in a text editor and change `#define USB_MIDI_SYSEX_MAX 64` to at least 79

(Location example: `c:\users\<username>\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.61.0\cores\usb_midi`)

6. Go back one directory to the `cores` folder and open the `teensy` folder and Edit `HardwareSerial.cpp`

7. Find and change the following two lines: (Note: `RX_BUFFER_SIZE` was `64` which is too small for long SysEx)
```
#define RX_BUFFER_SIZE 128 
#define SERIAL1_TX_BUFFER_SIZE 256
```
8. After saving the above files you can compile the project in your Arduino IDE (`Board:Teensy2.0` `USB Type:MIDI`).

---

## 🔌 Hardware Pinout (Basic Info - or if not using my board design)

Connect your components to the **Teensy 2.0** board according to the pinout layout below:


| Component 		| Pin 		| Teensy 2.0 Pin| Notes |
| :--- | :---: | :---: | ---: |
| **SSD1306 OLED**	| GND 		| GND		| (Check pinout as some OLEDs are backwards!)|
|			| VCC 5V	| VCC 5V	|	|
| 			| SCL 		| **5**  (PD0)	| I2C Clock (`Wire.h`)|
| 			| SDA 		| **6**  (PD1)	| I2C Data  (`Wire.h`)|
|			|		|		|	|
| **Rotary Encoder**	| Pin A  	| **1**  (PB1)	| Main rotation pin|
| encoder module used	| Pin B  	| *24**  (PE6)	| Main rotation pin|
|			| Switch 	| **0**  (PB0)	| Encoder push-button switch|
|			| VCC		| VCC		|	|
|			| GND		| GND		|	|
|			|		|		|	|
| **Vol POT 10k-100k**	| Wiper Pin 	| **12** (PD7)	| 'Analog' Output Control (Volume potentiometer)|
|   Right Pin		| VCC		| VCC		| (optional part)|
|   Left  Pin		| GND		| GND		|	|
|			|		|		|	|
| **Reset Button** 	| Switch Pin 	| **10** (PC7)	| System Reset trigger|
|			| GND		| GND		| (optional part)|
|			|		|		|	|
| **Reset to SCB55**	| Reset Line 	| **2**  (PB2)	| Hardware reset line sent to SCB-55 - pin 26|
|			|		|		|	|
| **Serial MIDI RX** 	| MIDI IN Line	| **RX (Pin 7)**| (PD2) Hardware MIDI input (from H11L1+470Ω pull-up)|
|			|		|		|	|
| **SCB-55 MIDI RX** 	| MIDI OUT Line	| **TX (Pin 8)**| (PD3) MIDI stream to Roland SCB-55 - pin 4|

 Note: 	Pin numbers are programming numbers, add 2 for actual pin number except 24 = pin30, 12 = pin19.
	If you want to use external power and USB, the Teensy 2.0 must be modified. (see Teensy MOD picture)
	The SCB-55 also requires a +/-12V and +5 supply to run correctly along with L/R audio out.

		MIDI Input Example:
<p float="left">
  <img src="/images/MIDI_Example.jpg" width="400" /> 
</p>
(Note: The Ultra-cheap USB MIDI cables, with the treble clef, will corrupt all SysEx messages.)

---

## 🎹 MIDI & Serial Routing Architecture
* **Serial RX (Receive) :** Dedicated to handling incoming **MIDI In** streams.
* **USB MIDI  (Bi-directional):** to/from Teensy 2.0 (MIDI actions are echoed from UI to host PC)
* **Serial TX (Transmit):** Dedicated to delivering processed **MIDI data to the SCB-55** waveblaster module.

---

## 💻 MENUS, Teensy 2.0 Vusb mod, Rev1 Diode Mod (important) and other pictures

 
	Arduino IDE Settings
  <img src="/images/Arduino_IDE_Setting.jpg" width="500" />
	 <b>Hardware</b> 
  <img src="/images/Board_Rev1.jpg" width="400" /> 
  <img src="/images/Schematic_Rev1.jpg" width="400" /> 
	 <b>User Interface</b> 

| | |
| :---: | :---: |
| <img src="/images/Menu1.jpg" width="300" /> | <img src="/images/Menu2.jpg" width="300" /> |
	 Important if using dual power (*Teensy MOD) 
  <img src="/images/Teensy2Mod.jpg" width="400" /> 
	 <b>Initial Prototype</b> 
  <img src="/images/Proto-SmallOLED.jpg" width="400" /> 
  <img src="/images/Proto-LargeOLED.jpg" width="400" /> 
 <b>If using rev1 hardware (update coming)</b> 
  <img src="/images/ToUseRev1.jpg" width="400" /> 
  <img src="/images/Rev1_LargeOLED.jpg" width="400" /> 


---

## 📄 Credits and other notes

This used: 

Arduino IDE 2.3.10 - [IDE](https://www.arduino.cc/en/software/),  [Teensy2.0](https://www.pjrc.com/teensy/card2a_rev5_web.pdf)

KiCAD 10.0 (Along with several Footprints, please let me know if I missed credit on any), [KiCad](https://github.com/KiCad)

Teensy 2.0 Footprint [XenGi](https://github.com/XenGi/teensy_library)

FreeCAD 1.1.1 (to be used for the 3D case) - [FreeCAD](https://www.freecad.org/)

along with several programs that were very helpful for debug and testing:

The excellent MIDI Player 6.5 by Zoltán Bacskó (Falcosoft) https://falcosoft.hu/

The very impressive Nuked-SC55 emulator which helped verify proper display & behaviour. [Nuked-SC55](https://github.com/nukeykt/Nuked-SC55) 

Along with Roland who created the SCB-55 GS Daughter Board that made this possible.

Other GS/GM daughter boards that use the wavetable connection should be electrically compatible, but the firmware is written for the SCB-55 and would need to be modified to control others properly.  Also the board and corner holes have been sized for the SCB-55 specifically.  Feel free to make your own modifications.

---

## 📜 License

This project is licensed under the **GNU General Public License v3 (GPLv3)**. See the `LICENSE` file in the root of this repository for the full legal text. You are free to share and adapt this code, provided all derivative works remain open-source under the same terms.

## ⚙️ **Third-Party Libraries:** This software compiles dependencies from the **Arduino Core Library Framework** (LGPL-2.1) and the **Adafruit GFX Engine** (BSD 2-Clause). Full attribution terms are listed inside the [`THIRD-PARTY-NOTICES.md`](./THIRD-PARTY-NOTICES.md) file.

---

Please Note:

This is Rev.1, there are some hardware errors (POT wired backwards & I should have used a WRA0512S (not A0512S))
also I'm just about out of code space in the Teensy 2.0, but it does seem to be working quite well (with the mod of adding an inline 1N5819 in front of the A0512S on the 5V line to drop +/-12V to a safe level).  This diode bodge sacrifices the dual colour LED that I was planning, but it wasn't correct anyhow. 
(Don't get AI to 'help' design hardware without verifying results!)

This does work fully from USB only (< 350mA), or from external power with 5 pin MIDI, when connected to USB the MIDI jack is still active and either can be used (one at a time as there is no flow control).  Connecting USB and external power should be safe* as I've used an ORing module to prevent voltage feedback/feedthrough, but currently you will lose USB connection with the host PC (not sure if this can be fixed, but a powered hub can be used for a weak USB port in place of the external power jack).  *As long as the Teensy's `Vusb` is cut and wired to the board!

I plan on addressing the current hardware errors on the board and will see if I can optimize the code further as I do have some inconsistencies and some duplication in the code.  I also have a few more features to add (making the POT re-assignable, and various settings saved to EEPROM, etc.) I would also like to implement the display of bank 126 (but doubt there will be enough code space for this).

Any modifications or suggestions are welcome, please remember that I don't do this professionally and I did enlist AI input / modification for some of my code, so it may not be the best implementation possible. 

Once again, this works for me, but I take no liability or responsibility if any thing blows up or burns down and you assume all responsibility for creating all or parts of this project.
