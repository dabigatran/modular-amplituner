# modular-amplituner
Thesis project: "I2S bus control using I2C and SPI protocols on the example of a modular audio-video system consisting of a receiver (tuner), an amplifier and a speaker system".

Abstract: The market for consumer devices for managing audio and video transmission is dominated by commercial solutions with a top-down imposed way of controlling and processing audio-visual data and no possibility of interference from the advanced user in the hardware part of the device as well as the software part. In addition, these systems are characterized by a lack of modularity, which consequently leaves the user without the possibility of a PC-like hardware upgrade. The goal of this project is to create a modular audio/video system consisting of an audio/video receiver, audio amplifier and stereo speaker system with software for home use, based on commercially available components. The system is planned to use universal I2C and SPI protocols to control individual modules and the I2S bus. One of the system's most distinctive features is its modularity. This refers to the ability to make changes to the system, both in its individual components and in the software, according to the individual audio needs of the advanced user (audio hobbyist) over the course of the system's use.

Functional requirements:
1.	the speaker system shall provide:
  a.	sound intensity of up to 100dB (SPL),
  b.	sensitivity of at least 84dB, 
  c.	frequency range of at least 50-20000Hz,
  d.	impedance range of 4-8 Ohms. 
2.	the audio amplifier shall provide a minimum power rating of 150 watts.
3.	the audio receiver shall provide support for:
  a.	at least one digital wired audio source,
  b.	one wireless audio source,
  c.	at least one analog audio source,
  d.	at least four digital audio-video sources, 
  e.	one digital audio-video output,
  f.	one analog audio output (to an amplifier),
  g.	audio volume control,
  h.	additionally  control: sound source, bass, treble, balance,
  i.	on and off status of the device,
  j.	control of "standby" state,
  k.	control is provided by:
    i.	two knobs with button functionality and a single button,
    ii.	infrared remote control with integration with the remote control of the Chromecast device.
  l.	display of the current status of the receiver, i.e. always the volume and the currently set additional functionality. 

 
Non-functional requirements:
1.	Speaker set:
  a.	two speaker columns (so-called monitors, bass-reflex type):
  b.	rated power up to 150W,
  c.	frequency range 45-24000Hz.
2.	Audio amplifier:
  a.	stereo,
  b.	class D,
  c.	rated power up to 150W/channel at THD+N < 0.01%,
  d.	switching power supply with EMI filter,
  e.	2x RCA audio input (left/right channel,
  f.	2x speaker outputs.
3.	Receiver (tuner):
  a.	4x HDMI inputs,
  b.	1x Spdif input,
  c.	1x Bluetooth 5.0 module with LDAC/aptx adaptive codec support,
  d.	1x input for audio via USBC (e.g. from a PC),
  e.	2x RCA type analog audio input (left/right channel),
  f.	1x HDMI output,
  g.	2x RCA-type analog audio output (left/right channel),
  h.	1x control unit: microcontroller (SoC),
  i.	audio digital-to-analog converter (DAC),
  j.	LCD display,
  k.	on/off switch,
  l.	volume control encoder with pushbutton function,
  m.	encoder with button function to set sound source, treble/low gain level, balance,
  n.	audio data bus: I2S,
  o.	control protocols: I2C, SPI,
  p.	programming language C.

Component inventory:
1.	Speaker set (based on SB Acoustics' open source BROMO project) :
  a.	2x SB26CDC-C000-4 tweeter, 
  b.	2x mid-bass speaker SB17CAC35-4 x2,
  c.	a set of components for two crossovers,
  d.	18 mm thick MDF board,
  e.	bass-reflex tunnel,
  f.	set of connectors,
  g.	veneer, veneer glue,
  h.	varnish
2.	Amplifier:
  a.	200W class D stereo amplifier module, THD+N < 0.01% in the range of 0.3W - 200W at 4Ω Ohms, 111dBA dynamic range (3eaudio model EAUMT-0260-2-B) based on           TPA3255 IC (Texas Instruments), 
  b.	dual LLC 500W, 46V switching power supply module (Lusya C1-009),
  c.	an EMI filter module (No-Name).
  d.	enclosure
3.	Receiver:
  a.	switching power supply 60W, 12V (No-Name).
  b.	2x step-down converter module (No-Name) based on TPS5430 chip (Texas Instruments).
  c.	linear stabilizer module (No-Name) based on 2x LM1084 +5V to 5A chip (Texas Instruments),
  d.	low-noise +5V to 500mA linear stabilizer module (No-Name) based on 2x LT3045 chip (Analog Devices),
  e.	low-noise +5V and -5V to 300mA linear stabilizer module (No-Name) based on LT3045 and LT3094 chips (Analog Devices),
  f.	SoC ESP32 WROVER DEV KIT microcontroller module (Espressif Systems),
  g.	MCP23017 I2C bus extender module (Waveshare 15391),
  h.	LCD1602 display module (No-Name), 
  i.	LCD controller module via I2C protocol based on PCF8574 (Philips Semicoductors),
  j.	DAC module based on PCM5102 chip (Burr-Brown),
  k.	Analog audio control module based on NJW1194 chip (New Japan Radio),
  l.	Bluetooth 5.0 stereo module based on QCC5125 chip (Qualcomm),
  m.	Digital signal converter (SPDIF) to I2S protocol module based on DIR9001 chip (Texas Instruments),
  n.	I2S signal switch module based on HC245 chip (ONSemi),
  o.	HDMI source and output control module (No-Name) based on the Lontium LT8641SXE chip (Lontium Semiconductors).
  p.	5V-3.3V logic level converter module (No-Name),
  q.	control module with two PEC11H series encoders (Bourns) and switch
  r.	inverter module with Schmitt circuit based on 74HC14 integrated circuit (Diodes Incorporated),
  s.	NPN transistors, resistors, capacitors, universal PCBs, wiring, RCA connectors, USB, Bluetooth antennas, heat sink
  t.	housing
