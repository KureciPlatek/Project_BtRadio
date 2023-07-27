# Bluetooth radio project

This project is about renewing an old (and wonderful) Phillips Hurricane De Luxe 50IC361.
It still uses loudspeaker, but inside is everything new.

The aim is also to have fun with new technologies and train HW/SW skills as well as project management.

Technology used:  
- Bluetooth chip RN52 (old but had it on a shelf that was getting dust for years)
- FM demodulator from Silicon Labs: si470x which is able to demodulate FM radio emission and decode RDS (Radio Data System)
- Epaper 5.8" from Waveshare, they actually fit almost perfectly where frequency panel was
- Rotary encoders to give "digital" signals to central/control unit
- RP2040 as central unit because it is cheap, has double core features and suits perfectly the need of this project
- KiCad for HW development
- CMake for software development
- git for versionning and project management

For the management part, full development is managed with a TICKETS.md textual file. It is split into 3 parts: SYS, HW and SW: SYStem, HardWare and SoftWare. No particular mechanical development was done so no section for that is made (it could exists to interface PCB into old Phillips radio though).

## Requirements:

- Raspberry pico sdk
- cmake
- gnu c compiler toolsuite

## Bluetooth module

## FM module

## Rotary encoders

## e-Paper

## Architecture

## Todo

