#!/bin/bash
sox clip.wav -c 1 -b16 -esigned clip.raw
./go
