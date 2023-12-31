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
|SYS_13| *Architecture* Add clean doxygen API documentation of drivers to architecture documentation |NEW|
|SYS_15| *Architecture* Add add link to requirements/constraints, everywhere in project description |NEW|


**Work In Progress issues:**  

|Issue| Description | state |
|-|-|-|
|SYS_11| *System* Integrate all (obviously) | WIP - Breadboard done, integration of RE, BT and EP done. FM module will be deactivated at the beginning |
|SYS_12| *Architecture* Write down all algorithms and interrupt routines in UML diagrams/pseudo code |NEW|


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
|SYS_14| *Architecture* Add application main usecases and link them to runtime view of software architecture |DONE - Add of Usecase diagram and its corresponding behavioral diagrams|
|SYS_9| *HMI* Check that buttons of Phillips radio are good: maybe button on far right is just push and not push/hold | DONE - Button on far right is only push but not hold. Start of radio is now on main button II |


**Rejected issues**

|Issue| Description | state |
|-|-|-|
|SYS_1| *Use of a battery* To use the radio without any power supply, a battery must be added. It implies however some constraints on the available voltage on the PCB. This limits the choice of audio amplifier available. Create a votlage which is bigger than the one from power supply is quite complex and may lead to development complexity | REJECTED - Too complex for Mark I futur use: RP2040 Plus Based (contains a Lithium manager) |
|SYS_8| *HMI* Add RealTime clock and Date for more infos on screen | REJECTED - will take too much time |
|SYS_9| *Memory* Add external flash like a W25Q16JV (SPI, 133 MHz) or an M25P16 (SPI, 75 MHz) | REJECTED - Should be fine without. Otherwise, our system is too complex |
|SYS_5| *Architecture* Create block diagram of main element with their interfaces | REJECTED - Carry on with project and get it done |
|SYS_10| *Proj_DVA* Add AD8248 circuit and PCB to project | REJECTED - one system at a time |


***
## SOFTWARE ISSUES

**New issues:**  

|Issue| Description | state |
|-|-|-|
|SFW_24 |*Doc* Cleanup and generate Doxygen API | NEW |
|SFW_29 |*FM_module* FM demodulator has a strange reaction to seek or tune and can't find any channel/station | NEW |

**Work In Progress issues:**  

|Issue| Description | state |
|-|-|-|
|SFW_14 |*FM_module* Add behavioral in application | WIP - half commented out |
|SFW_7 |*Architecture* Describe code as a waiting state machine to recieve "internal interrupts", which are tokens | WIP - partially implemented, they way inputs from modules are processed should be checked |
|SFW_3 |*Architecture* Cleanup git repo, create clean architecture of project | WIP - cleanup code for useless and commented lines |
|SFW_28| *BT_module*, *FM_module* Deactivate interrupts of module when not in use | WIP - only UART and GPIO interrupts are activable/deactivable |

**Resolved issues:**

|Issue| Description | state |
|-|-|-|
|SFW_1 |*e-Paper* Add test and interface to RpPico with assigned pins | DONE - use of example in /home/jeremiegallee/Pico_epaper_example/ |
|SFW_10 |*Coding* synchronize naming with architecture (prefixes) bt_rn52 for bluetooth module and fm_si470x for FM module, re for rotary encoders and epaper for HMI | DONE - set bt_, fm_, re_ and ep_ prefixes |
|SFW_8 |*BT mode* Create high level API to communicate with RN52 module through UART and AT commands | DONE - already implemented |
|SFW_18 |*FM mode* Restructure fm_application and fm_api as they have common role and SOLID principles not fully respected (call only from one place of the module, not 2) | DONE - renamed functions prototypes and file names |
|SFW_5 |*Architecture* Create main state machine and its sub state machines | DONE - Main state machine processes new inputs and execute corresponding actions |
|SFW_9 |*BT mode* Add feature to print "ready to pair", "connected" and so on to epaper screen | DONE - keep it simple and just check if connection bit of rn52 status registers is still set |
|SFW_25 |*EP_module* EP_module provided API can't load new lines and keep the old ones. Way to use it should be different | DONE - call Paint_ClearWindows() to clean one line only |
|SFW_4 |*Architecture* Create RotaryEncoder interrupts | DONE - Test success |
|SFW_13 | *BT mode* Add GPIO2 interrupt (which implies to send Q to radio) | DONE - Q is sent to RN52 after GPIO2 interrupt |
|SFW_15 | *Main* Take care of radio project init and selection of mode with GPIOs | DONE - GPIOS which requires interrupt are gathered together in hal_radio module |
|SFW_19 |*e-Paper* Add clean API for rest of the application | DONE |
|SFW_20 |*All* Make project compile. Maybe restart full project CMakes as they are strange | DONE - use simple CMake structure |
|SFW_16 |*REx* Add timeout before other GPIOs may "react". Aim is to entprellen | DONE - Not perfect but enough for requirements |
|SFW_21 |*RP2040* Check this stuff with double core. We may have pbm with interrupts affected to a core | DONE - Only core0 is called. Nothing on core1 right now |
|SFW_17 | *BT mode* Add processing function to analyse retrieved data from RN52's UART comm | DONE - RN52's comm had to be hugely cleaned up |
|SFW_27 |*EP_module* Check why is epaper having this cathodic tube like effect (imageBuffer not all 0?) | DONE - had to clean heap where image buffer was saved |
|SFW_26 |*EP_module* Add screenClear() procedure before shuting down. Otherwise, pixels will stay active and may damage epaper | DONE - Use of ep_deactivate() when no HW mode is selected |
|SFW_2 |*FM_module* Strange freeze after first seek up/down. Debug with PicoProbe | DONE - A too small array was given and it was overwriting out of bonds on the stack |


**Rejected issues**

|Issue| Description | state |
|-|-|-|
|SFW_12 |*FM mode* Manage FM as a handle too | REJECTED - use getters and setters instead |
|SFW_11 |*REx* Add delay between each rotary A or B key | REJECTED - Control of RE is not perfect but enough for now |
|SFW_22 |*RP2040* [Optional] use of FreeRTOS for multitasking | REJECTED - Do it in a second version |
|SFW_23 |*Structure* Modify and clean hal_main for GPIOs IRQ as it is too quick and dirty | REJECTED - Do it in a second version |

***
## HARDWARE ISSUES

**New issues:**  

|Issue| Description | state |
|-|-|-|


**Work In Progress issues:**  

|Issue| Description | state |
|-|-|-|


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
|HDW_33| *PCB* LM317: move so that it may be mounted horizontally | DONE - all pieces moved |
|HDW_32| *Powersupply* Put potentiometer for voltage regulator | DONE - added in schematic and PCB |
|HDW_31| *Powersupply* Add potential DC-DC downconverter for 12V battery pack | DONE - added in schematic and PCB |
|HDW_34| *PCB* Move all button resistance down to few mm as they will disturb mechanical place of button holders | DONE - all resistance moved |
|HDW_36| *PCB* Add boarder lines for cut | DONE - Had to remove fully drawn circle from edge.cut layer |
|HDW_25| *PCB* Install holes for screwing/monting on old radio | DONE - add mounting holes for PAM8302 too |
|HDW_38| *All* Fix UART communication wires. BT_Rx is actually on GPIO4 and BT_Tx on GPIO5 | DONE - modify schematic and PCB |
|HDW_41| *Routing* BT, EP and FM modules have been wrongly routed. Correct PCB and Software | DONE - changed communication lines of UART and I2C for BT, FM and EP modules |
|HDW_45| *Powersupply* Remove 12V transformer, go only on 5V | DONE - removed in schematic and PCB |
|HDW_39| *Schematic*  RP2040 actually has very low internal pull-up/down. 100k is way too high and signal at RP2040 pin is too small. Check pull-up and pull down resistance of 10k or 4,7k are ok | DONE - in commit 45d533c HDW_41 |
|HDW_44| *PCB* Check that push buttons are good, only on dry contact way, but check that a pushed button is either high or low active, without any perturbations | DONE - push buttons are gnd in normal position and to Vcc when pushed |
|HDW_42| *PCB* Move mechanical place of push buttons, they are biased of few millimeters, add a system to fix/set vertical place of mechanical buttons at production (set/adapt it at mounting) | DONE dimensions were measured on final target for mounting |
|HDW_50| *PCB* Check mechanical place of mounting screws if they fit to the right place | DONE mouhting screw are now in the right place mut may be small |
|HDW_49| *PCB* Check that mechanical place for eval-boards fits | DONE - all good |
|HDW_51| *Schematic* Change on/off button to connector II or more as I is not push/hold but just push | DONE - moved to push button II and update of PCB |
|HDW_54| *All* Update BOM and buy parts | DONE - had to change C15 and C16 from 0805 to 1210 footprints |
|HDW_53| *PCB* Modify layout of preamplifier and switch to improve gnd path | DONE - improved gnd panel and placement |
|HDW_55| *PCB* Improve grounding of F and B copper ground panes by connecting push buttons jumper unused vias to ground | DONE |
|HDW_37| *PCB* Send PCB to production, gather last required components, sold everything together | DONE - go to V1.1, V1.0 is not good |
|HDW_7 | *Audio ampl* Order and test the TS5A2234 analog audio switch | DONE - works fine |
|HDW_19| *RP2040* Check for powering if all good | WIP - test a 3V3 on Vsys on power supply (no microUSB load) | DONE - seems to work |
|HDW_22| *Schematic* Check that with the actual schematic connection, RP2040 can control epaper | DONE - routing is good |
|HDW_28| *Powersupply* Test that LM317 do bring enough power to supply board at full need | DONE - Real men test only in production |
|HDW_30| *All* Create clean mock-up with breadbord and elements check that all HW works | DONE - Real men test only in production |
|HDW_52| *test* Check that soldered components on Bluetooth and RP2040 are good. Solder all and verify | DONE - Real men test only in production |
|HDW_43| *PCB* Re-write mechanical edges of PCB as the actual do not fit in radio | DONE - printed on paper and V1.1 is ready for production |


**Rejected issues**

|Issue| Description | state |
|-|-|-|
|HDW_1| *Linked do SYS_1* Battery management system must be found as well as a battery. A reloadable one must be chosen which allows an audio amplifier to work with 10 Watts minimum | REJECTED - too complex |
|HDW_14| *Modularity* Add link to ClassA amplifier and ClassD. At PCB creation, HMI+RP2040 parts will be doubled, Class A and Class D not. Split PCBs into two parts, one with Audio ampl, one with the rest. | REJECTED - too complex, maybe a powerful class D will be used like the TPA3223 |
|HDW_5 |*Board* Add connectors for exclusive push buttons of vintage and phillips radios | REJECTED - Double with ticket HDW_4 |
|HDW_11| *Class A audio amplifier* check that all component from current's driving class A amplifier exists | REJECTED - Class A will take too much time to develop |
|HDW_13| *Breakout boards* Some of the modules are from breakout boards. Add mechanical holder for them | REJECTED - No need for that. Header will be fine |
|HDW_26| *PCB* - *Proj_DVA* Add PCB of AD8428 evaluation board | REJECTED - one project at a time |
|HDW_29| *PCB* Add testpoint on many lines as it will help modify lines if needed | REJECTED, will find solutions without |
|HDW_27| *Schematic* - *Proj_DVA* Add schematics of AD8428 evaluation board. Design different topologies to select best one | REJECTED - one project at a time |
|HDW_40| *Powersupply* DC-DC converter 12 to 5 volts from MURATA is not always available and Wuerth Elektronik provides wome acceptable solutions. Add branch or schematic to manage both configurations | REJECTED - Do it in a second version |
|HDW_46| *Powersupply* Add SparkFun Battery Babysitter based on BQ24075 and BQ27441-G1A | REJECTED - finish the project |
|HDW_47| *Powersupply* Check if SparkfunBattery Babysitter is enough for 3.3V power supply | REJECTED - finish the project |
|HDW_48| *Powersupply* Add LiPo battery 1250 mAh | REJECTED - finish the project |

