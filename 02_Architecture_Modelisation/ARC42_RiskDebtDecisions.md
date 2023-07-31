# Architecture decisions, risks and technical debts

This chapter gathers technical decisions made, why it was decided like that and so on. Some of those decisions were already half explained earlier. This chapter regroups them cleanly

## Technical (architecture) decisions

- Program with mainly be written in C as the whole Pico SDK is available in C, as well as other third party source code. C++ is quite strong, but a well used C is as powerful as C++ and presents the advantage of having a really close to hardware language (even if C++ do provide "low layers", the intensive use of object oriented programming may become an anti-pattern).  

- No AirPlay. Maybe in a second version. But in a first one, use Bluetooth is plenty enough for a fun use of this radio. Many nomad radios also implement only Bluetooth.  

- No Bluetooth low energy. Simply because I wanted to use this old chip I have and because it is enough for the need presented.  

- No AM demodulation. Simply because it is almost never used anymore. Even if it still possible to capture radiowaves from the other side of Europa.  

- The old electronic board will be fully replaced. A first try, using the old AB audio amplifier and FM demodulation was not a success as the technologies doesn't perfectly matches. Indeed, an annoying PIIIIIP was to be heard when playing from Bluetooth stream, using old AB amplifier. Debug it on a hardware point of view may be quick, as it may be VERY long (contradicting organizational constraints).

- No RTOS in a first version, running program on one core seems good. But having fun with hard or soft RTOS with preemptive tasking, is the next step in pipe. For example, RDS decoding could be done on Core1, while rest of code is on Core0. Because RDS is a long parallel task, which may require quite the processing to correct errors.

- Hardware sub-modules are mainly pre-made PCBs, from either Sparkfun or other producers. It reduces HW instability risks.

## Risks

- E-paper has a disadvantage in outdoor, as it doesn't like the sun (as I read). It may be possible to have to switch to another screen, which is more outdoor oriented.

- Bluetooth chip is acutally obsolete and may have to be changed when the old Bluetooth 2.1 will not be supported on new Smartphones. Indeed, Bluetooth evolve quite fast and it could be in a close future that this project can't connect to new Smartphones anymore. The risk is medium, but correction is quite easy, as Bluetooth chip will be mounted on a unpluggable PCB, with a lot of pins.

- Mechanical fit: there is no documentation (or you have to pay) for mechanical dimensions of PCB. This PCB has however to be fixed on ŕadio case, so that push buttons on the top works. And also, a moving PCB in the boxing is not quite recommended. Fixing hole may have to be done/adapted manually, after PCB is produced.

## Technical debt

- A clean RDS decoding, with error correction still have to be done, so that FM module will be fun to use. This part is not tricky, but requires a little bit of time to be stable.

- Clean and powerful use of RP2040 double core system still to be done. Exploit it with FreeRTOS.

- Rotary encoders are actually of poor quality and doesn't present a good A and B signals. Sometimes turing directly procures an indirect turn. Even with resistors and capacitors, on oscilloscope signals aren't always clean.

- Footprint of different modules is quite huge and PCB is on a mechanical point of view, almost full. RP2040 pins are all used and add other devices may be difficult. Remove "sub-pcb" modules and put RP2040, as well as FM, BT modules could be directly installed on PCB.

[< prev (Architecture design)](./ARC42_architectureMain.md)