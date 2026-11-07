# Cactus-Compute Whistle as pure C

This is a low-effort distillation of the excellent reverse-engineering
effort of Andris Gauračs on the Cactus Whistle speech-to-text engine.

The code here is mostly taken from:

https://github.com/andrisgauracs/whistle-esp32

with a view to running on Linux desktop rather than esp32 platform.

All I've done is remove the ESP32 references and added a main.c shim
to read in the standard whistle.cact model file and a 'clip.raw' audio file 
(just a clip.wav file in raw PCM format with the WAV header removed)
and generate a transcript of that.

## Compiling

just use 'make'. A binary 'go' is generated. 

## Running

./go

It should generate the text:

A tiger and a mouse were walking in a field
and they saw a big lump of cheese lying on the ground.

I've found that it runs at pretty much the same speed as the provided
X86-64 'needle' binary when compiled on a Debian-13 VM. (about 11x realtime
on my Zen-2 desktop)

There's a 2-line wav_to_raw.sh that uses the Sox application to make the
clip.raw file from a clip.wav file.


(Code provided for reseach purposes mainly. Only works for fairly short clips)
