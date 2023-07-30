# Vintage radio with Bluetooth

This project is about renewing an old (and wonderful) Phillips Hurricane De Luxe 50IC361.
It still uses loudspeaker, but inside is everything new. It becomes new and modern audio stream source

The aim is also to have fun with new technologies and train HW/SW skills as well as project management. 

## The radio:  
![Philips Hurricane de Luxe](./06_Doc/pictures/radio_originalpicture.png "Philips 50IC360")

## Technology used:  
- Bluetooth chip RN52 (old but had it on a shelf that was getting dust for years)
- FM demodulator from Silicon Labs: si470x which is able to demodulate FM radio emission and decode RDS (Radio Data System)
- Epaper 5.8" from Waveshare, they actually fit almost perfectly where frequency panel was
- Rotary encoders to give "digital" signals to central/control unit
- RP2040 as central unit because it is cheap, has double core features and suits perfectly the need of this project
- KiCad for HW development
- CMake for software development
- git for versionning and project management
- Use ARC42 documentation format to present project and explain architecture

For the management part, full development is managed with a TICKETS.md textual file. It is split into 3 parts: SYS, HW and SW: SYStem, HardWare and SoftWare. No particular mechanical development was done so no section for that is made (it could exists to interface PCB into old Phillips radio though).

All details here:
- Introduction, goals and constraints: [ARC42_IntroAndGoals](./00_Requirements_Inputs/ARC42_IntroAndGoals.md)
- Contextes and solution strategy: [ARC42_SolStrat](./01_Analysis/ARC42_ContextAndScope.md)
- Architecture description: [ARC42_Architecture](./02_Architecture_Modelisation/ARC42_architectureMain.md)
- Project decisions, risks and technical debts: [ARC42_DecisionsRisksDebts](./02_Architecture_Modelisation/ARC42_RiskDebtDecisions.md)

## Requirements:

- Raspberry pico sdk
- cmake
- gnu c compiler toolsuite

