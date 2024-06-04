# modular-amplituner
Project: "I2S bus control using I2C and SPI protocols on the example of a modular audio-video system consisting of a receiver (tuner), an amplifier and a speaker system".<br>
<br>
Abstract: The market for consumer devices for managing audio and video transmission is dominated by commercial solutions with a top-down imposed way of controlling and processing audio-visual data and no possibility of interference from the advanced user in the hardware part of the device as well as the software part. In addition, these systems are characterized by a lack of modularity, which consequently leaves the user without the possibility of a PC-like hardware upgrade. The goal of this project is to create a modular audio/video system consisting of an audio/video receiver, audio amplifier and stereo speaker system with software for home use, based on commercially available components. The system is planned to use universal I2C and SPI protocols to control individual modules and the I2S bus. One of the system's most distinctive features is its modularity. This refers to the ability to make changes to the system, both in its individual components and in the software, according to the individual audio needs of the advanced user (audio hobbyist) over the course of the system's use.<br>
<br>
Functional requirements:<br>
1.	the speaker system shall provide:<br>
  a.	sound intensity of up to 100dB (SPL),<br>
  b.	sensitivity of at least 84dB, <br>
  c.	frequency range of at least 50-20000Hz,<br>
  d.	impedance range of 4-8 Ohms. <br>
2.	the audio amplifier shall provide a minimum power rating of 150 watts.<br>
3.	the audio receiver shall provide support for:<br>
  a.	at least one digital wired audio source,<br>
  b.	one wireless audio source, <br>
  c.	at least one analog audio source,<br>
  d.	at least four digital audio-video sources, <br>
  e.	one digital audio-video output,<br>
  f.	one analog audio output (to an amplifier),<br>
  g.	audio volume control,<br>
  h.	additionally  control: sound source, bass, treble, balance,<br>
  i.	on and off status of the device,<br>
  j.	control of "standby" state,<br>
  k.	control is provided by:<br>
    i.	two knobs with button functionality and a single button,<br>
    ii.	infrared remote control with integration with the remote control of the Chromecast device.<br>
  l.	display of the current status of the receiver, i.e. always the volume and the currently set additional functionality. <br>
<br>
Non-functional requirements:<br>
1.	Speaker set:<br>
  a.	two speaker columns (so-called monitors, bass-reflex type):<br>
  b.	rated power up to 150W,<br>
  c.	frequency range 45-24000Hz.<br>
2.	Audio amplifier:<br>
  a.	stereo,<br>
  b.	class D,<br>
  c.	rated power up to 150W/channel at THD+N < 0.01%,<br>
  d.	switching power supply with EMI filter,<br>
  e.	2x RCA audio input (left/right channel,<br>
  f.	2x speaker outputs.<br>
3.	Receiver (tuner):<br>
  a.	4x HDMI inputs,<br>
  b.	1x Spdif input,<br>
  c.	1x Bluetooth 5.0 module with LDAC/aptx adaptive codec support,<br>
  d.	1x input for audio via USBC (e.g. from a PC),<br>
  e.	2x RCA type analog audio input (left/right channel),<br>
  f.	1x HDMI output,<br>
  g.	2x RCA-type analog audio output (left/right channel),<br>
  h.	1x control unit: microcontroller (SoC),<br>
  i.	audio digital-to-analog converter (DAC),<br>
  j.	LCD display,<br>
  k.	on/off switch,<br>
  l.	volume control encoder with pushbutton function,<br>
  m.	encoder with button function to set sound source, treble/low gain level, balance,<br>
  n.	audio data bus: I2S,<br>
  o.	control protocols: I2C, SPI,<br>
  p.	programming language C.<br>
