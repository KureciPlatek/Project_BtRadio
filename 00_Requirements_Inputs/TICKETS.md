# Project Bluetooth radio

## INTRO:

In this file are listed all tickets/todo to be checked and done. Another program should be used but it will suffice for now.

It is sorted by:

|level|description|Identification prefix|comments|
|:-:|:-|:-:|:-|
|system|means application, behavior, user experience, non technical topics|SYS_||
|software|pure software, can be bare metal, like driver, to scheduling items|SFW_||
|hardware|everything that is outside the DSP and are linked to electronic development|HDW_||

*__Note:__  
the issue identification is a prefix followed by a number which is the issue identification number.   
For example: SYS_14 is the system level issue 14*

*__Note:__  
When closing an issue (rejected or resolved), please add the SHA-1 number from git to identify it and track it back.*


***
## SYSTEM ISSUES

**New issues:**  

|Issue| Description | state |
|-|-|-|
|SYS_9| *HMI* Check that buttons of Phillips radio are good: mayb button on far right is just push and not push/hold | NEW |
|SYS_10| *Proj_DVA* Add AD8248 circuit and PCB to project |NEW|
|SYS_11| *System* Integrate all (obviously) |NEW|

**Work In Progress issues:**  

|Issue| Description | state |
|-|-|-|


**Resolved issues:**

|Issue| Description | state |
|-|-|-|
|SYS_3| *FM tuner* Check for a good AM/FM demodulator, some integrate RDS and RBDS, like Si4703 (look for Sparkfun breakout board) | DONE - use of Si4703 breakout board |  
|SYS_6| *HMI* Use of ePaper screen with RP2040 Pico https://www.berrybase.de/5-83-zoll-e-ink-display-modul-fuer-raspberry-pi-pico-648-480-schwarz/weiss | DONE |
|SYS_2| *Multiple PCB project* As some Bluetooth radio may stay on furnitures, like the vintage one, a direct plug to 220V network and a stronger audio amplifier is possible. How to manage two kind of PCB should be decided | DONE - create two PCBs |
|SYS_7| *Audio amplification* Check if Class D is sufficient and remove the TPA6112A2 | DONE - keep TPA6112A2 as it is wonderfully powerful with it |
|SYS_4| *HMI* List all input and features that system shall have: AM/FM switch, Bluetooth pair and connect, maybe a small screen | DONE - AM/FM in option, rest is all integrated |
|SYS_5| *HMI* Test and assignate available push button and rotary button of Phillips to interface of RN52 | DONE - Look at document  Architecture->Behavioral->hmi_description.md    |
|SYS_8| *Project versioning* Add a github repository | DONE - but lose of old git repo history |


**Rejected issues**

|Issue| Description | state |
|-|-|-|
|SYS_1| *Use of a battery* To use the radio without any power supply, a battery must be added. It implies however some constraints on the available voltage on the PCB. This limits the choice of audio amplifier available. Create a votlage which is bigger than the one from power supply is quite complex and may lead to development complexity | REJECTED - Too complex for Mark I futur use: RP2040 Plus Based (contains a Lithium manager) |
|SYS_8| *HMI* Add RealTime clock and Date for more infos on screen | REJECTED - will take too much time |
|SYS_9| *Memory* Add external flash like a W25Q16JV (SPI, 133 MHz) or an M25P16 (SPI, 75 MHz) | REJECTED - Should be fine without. Otherwise, our system is too complex |
|SYS_5| *Architecture* Create block diagram of main element with their interfaces | REJECTED - Carry on with project and get it done |

***
## SOFTWARE ISSUES

**New issues:**  

|Issue| Description | state |
|-|-|-|
|SFW_3 |*Architecture* Cleanup git repo, create clean architecture of project |NEW|
|SFW_5 |*Architecture* Create main state machine and its sub state machines | NEW |
|SFW_6 |*Architecture* Write down all algorithms and interrupt routines in UML diagrams/pseudo code | NEW |
|SFW_9 |*BT mode* Add feature to print "ready to pair", "connected" and so on to epaper screen | NEW |
|SFW_13 |*BT mode* Add GPIO2 interrupt (which implies to send Q to radio) | NEW |
|SFW_15 |*Main* Take care of radio project init | NEW |
|SFW_16 |*REx* Add timeout before other GPIOs may "react". Aim is to entprellen | NEW |

**Work In Progress issues:**  

|Issue| Description | state |
|-|-|-|
|SFW_2 |*FM_module* Strange freeze after first seek up/down. Debug with PicoProbe | WIP |
|SFW_4 |*Architecture* Create RotaryEncoder interrupts | WIP - need to test |
|SFW_14 |*FM_module* Add behavioral in application | WIP - half commented out |
|SFW_11 |*REx* Add delay between each rotary A or B key | WIP - must be tested |
|SFW_7 |*Architecture* Describe code as a waiting state machine to recieve "internal interrupts", which are tokens | WIP - partially implemented, they way inputs from modules are processed should be checked |
|SFW_17 | *BT mode* Add processing function to analyse retrieved data from RN52's UART comm | WIP - processing of received chars still to do |

**Resolved issues:**

|Issue| Description | state |
|-|-|-|
|SFW_1 |*e-Paper* Add test and interface to RpPico with assigned pins | DONE - use of example in /home/jeremiegallee/Pico_epaper_example/ |
|SFW_10 |*Coding* synchronize naming with architecture (prefixes) bt_rn52 for bluetooth module and fm_si470x for FM module, re for rotary encoders and epaper for HMI | DONE - set bt_, fm_, re_ and ep_ prefixes |
|SFW_8 |*BT mode* Create high level API to communicate with RN52 module through UART and AT commands | DONE - already implemented |

**Rejected issues**

|Issue| Description | state |
|-|-|-|
|SFW_12 |*FM mode* Manage FM as a handle too | REJECTED - use getters and setters instead |

***
## HARDWARE ISSUES

**New issues:**  

|Issue| Description | state |
|-|-|-|
|HDW_25| *PCB* Install holes for screwing/monting on old radio | NEW |
|HDW_26| *PCB* - *Proj_DVA* Add PCB of AD8428 evaluation board | NEW |
|HDW_27| *Schematic* - *Proj_DVA* Add schematics of AD8428 evaluation board. Design different topologies to select best one | NEW |
|HDW_28| *Powersupply* Test that LM317 do bring enough power to supply board at full need | NEW |


**Work In Progress issues:**  

|Issue| Description | state |
|-|-|-|
|HDW_19| *RP2040* Check for powering if all good | WIP - test a 3V3 on Vsys on power supply (no microUSB load) | WIP |
|HDW_7 | *Audio ampl* Order and test the TS5A2234 analog audio switch | WIP |
|HDW_22| *Schematic* Check that with the actual schematic connection, RP2040 can control epaper | WIP |

**Resolved issues:**

|Issue| Description | state |
|-|-|-|
|HDW_3| *Programming protection* To avoid too many components on board. Use of Waveshare USB to TTL. But it must be set to 3.3V because will be directly connected to RN52 power supply| DONE |
|HDW_2| *Programming interface* The RN52 must be programmed through an UART interface. The board must be disabled from voltage, so that only Waveshare USB to TTL may power the board and program it through UART | DONE - all RN52 boards to last firmware and UART connector added |
|HDW_9 | *Structure* Organize Kicad project into sheets, one sheet per module of board | DONE - use hierarchical sheet |
|HDW_12| *Class A audio amplifier* generate current driven class A audio amplifier circuit | DONE - but will be unused firstly |
|HDW_4 |*Board* create footprints for buttons of old Phillips radio | DONE - footprints are simple 2.57 mm pitch header footprints |
|HDW_14| *Structure* Rename all labels for a hierarchical and structured naming. Do it with sheets too | DONE |
|HDW_6 |*Board* Check for debug interface with RaspPico debug probe and BlackMagicProbe | DONE - Picoprobe will be used instead of BMS. So install Picoprobe connector with BMS style|
|HDW_8 |*Powersupply* Find circuit to connect to 220 VAC directly. It will provide the advantage to generate +5V, +3V3 and +30V for bigger audio amplifiers | DONE - Use an LM317 and buy/use a "good ol'" 12V power supply. RP2040 supplied through Vsys (main power input) |
|HDW_15| *Schematic* Add all values to passive R and C elements | DONE |
|HDW_10| *Housing* Create connectors for each button/selection on body housing | DONE - NO/NC connection are available on Phillips 50IC361 datasheet |
|HDW_16| *Schematic* Add e-paper display connector to board | DONE - subsheet created with connectors |
|HDW_18| *Schematic* Change power management symbols for ERC check success | DONE - change earth to GND symbol and issue lot of minor changes |
|HDW_20| *PCB* Check that all boards are well placed (direction of pins and connectors) as well as they have enough mechanic place | DONE - had to increase brightness of RN52 Sparkfun board |
|HDW_21| *PCB* Check size of holes for buttons header. Maybe the legacy component doesn't fin inside it | DONE - They fit, but only from one side |
|HDW_24| *PCB* Add name and version on board | DONE |
|HDW_17| *Schematic* Add list of elements that are not on PCB, like Jack barrel and heat sink | DONE - ordered heat sink and USB plug |
|HDW_23| *PCB* CHeck minimal via and wire brigthness allowed by JLCPCB | DONE - JLCPCB offers up to 0,127 mm wire thickness |


**Rejected issues**

|Issue| Description | state |
|-|-|-|
|HDW_1| *Linked do SYS_1* Battery management system must be found as well as a battery. A reloadable one must be chosen which allows an audio amplifier to work with 10 Watts minimum | REJECTED - too complex |
|HDW_14| *Modularity* Add link to ClassA amplifier and ClassD. At PCB creation, HMI+RP2040 parts will be doubled, Class A and Class D not. Split PCBs into two parts, one with Audio ampl, one with the rest. | REJECTED - too complex, maybe a powerful class D will be used like the TPA3223 |
|HDW_5 |*Board* Add connectors for exclusive push buttons of vintage and phillips radios | REJECTED - Double with ticket HDW_4 |
|HDW_11| *Class A audio amplifier* check that all component from current's driving class A amplifier exists | REJECTED - Class A will take too much time to develop |
|HDW_13| *Breakout boards* Some of the modules are from breakout boards. Add mechanical holder for them | REJECTED - No need for that. Header will be fine |


