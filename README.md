
# IR Remote Control Project

  <img src="https://i.imgur.com/cf2U9kA.jpeg" width="45%" style="display:inline-block;"/>

  

## Description

  

This project implements a custom infrared (IR) remote control system on the Arduino platform with an SD card interface and an OLED display (using the U8x8 library). It features a file-explorer-like menu navigated exclusively by buttons, allowing users to browse and select IR commands stored as binary files on the SD card and transmit them through an IR LED.

  

The IR signals are stored in a compact binary format derived from the open-source IR database [irdb](https://github.com/probonopd/irdb), adapted to optimize memory use and playback on embedded hardware.

  
  
  

## Requirements

  

* Arduino board with sufficient memory (NOTE: **does not work on Arduino Uno due to limited memory**)

* IR LED connected to Arduino digital output

* SD card module

* OLED display compatible with U8x8 library (e.g., SSD1306)

* Five navigation buttons (Up, Down, Left, Right, Enter) wired with pull-down resistors



## Dependencies

  

* [IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) library for IR transmission

* [SdFat](https://github.com/greiman/SdFat) or Arduino built-in SD library for SD card access

* [U8x8lib](https://github.com/olikraus/u8g2) minimalistic library for OLED display

  
  
  

## Getting Started

  

1. Prepare your SD card by copying the contents of the `sd` folder. It contains conveniently structured data with the format `/first_letter/brand_name/device_type/device_model/function.bin`.

NOTE: the program acts as a file explorer, so the folder hierarchy is flexible.

  

2. Load the Arduino program onto the Arduino board.

  

3. Insert the SD card and power the device.

  

4. Use the five navigation buttons to browse directories and files on the OLED display.

  

5. Select an IR command `.bin` file to transmit the corresponding IR signal via the IR LED.

  

6. Volia works like magic!

  
  

## Usage

   <img src="https://i.imgur.com/7NapIBv.jpeg" width="45%" style="display:inline-block;"/>

* **UP/DOWN** - go one element up or down

* **LEFT/RIGHT** - go a whole page up or down

* **MIDDLE** - enters the currently selected directory

* **Return** - places you back in the root folder (always the top option)

  
  

## Binary File Format

  

* Each `.bin` file contains a sequence of 16-bit little-endian unsigned integers representing IR pulse and space durations in microseconds.

* Timings alternate between 38kHz PWM ON and PWM OFF intervals, encoding the IR protocol waveform.

* This format allows efficient storage and fast transmission by the Arduino.

  
  
  

## IR Database

  

* Original raw IR timings from [irdb](https://github.com/probonopd/irdb) were converted from text to compact binary for reduced SD card space usage and faster read times.

* Filenames were sanitized: commas and unsupported characters replaced with hyphens to comply with Arduino filesystem constraints.

* Timing values normalized and rounded to match the Arduino timer resolution and protocol expectations.

* Protocol naming and signal encoding adapted to align with the project's IR transmission code.

  
  
  

## Supported Protocols

  

The following IR protocols are supported by the encoding system, mapped from the irdb protocol names:

  

* aiwa

* blaupunkt

* denon, Denon-K (denon-k, denon{1}, denon{2})

* dgtec

* DishPlayer\_Network (dish\_network, dishplayer)

* emerson

* f12

* fujitsu

* GI4dtv (g.i.4dtv)

* GI\_cable (g.i.cable)

* jvc (jvc, jvc-48, jvc{2})

* Jerrold

* lumagen

* mce

* Mitsubishi

* nec (nec, nec1, nec1-f16, nec1-rnc, nec1-y1, nec1-y2, nec1-y3, nec2, nec2-f16)

* NECx1, NECx2

* Nokia32 (nokia, nokia32)

* panasonic, panasonic2, panasonic\_old

* pioneer

* Proton

* rc5, rc5-7f

* rc6, rc6-6-20

* rca, rca(old), rca-38, rca-38(old)

* Samsung20, Samsung36

* sharp, sharpdvd, sharp{1}, sharp{2}

* Sony12, Sony15, Sony20

* streamzap

* Teac-K

* Thomson, Thomson7

* Tivo-Nec1 (tivo unit=0 to tivo unit=9)

* XMP (xmp, xmp-1, xmp-2)

  

# License

MIT License
