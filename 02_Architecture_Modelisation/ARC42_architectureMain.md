# Architecture description

This chapter describes the main architecture, first as a system view, complying to all requirement, quality goals and constraints.
Then it is split into Software, Hardware and Mechanical topics and architectures.

All issues and tickets about hardware and software topics are listed in [TICKETS](../00_Requirements_Inputs/TICKETS.md) file. As mechanical topics are rare and don't require many effort. No mechanical tickets are written.


## System architecture structure design

![System architecture](./ARC42_architectureSystem.svg "System architecture")

|Module| Purpose/Description| Interface|Technology| Nickname |
|:-:|:-|:-|:-|:-|
|Central Unit| Contains the software, interfaces almost all other modules and is the main entity of this system| All possible interfaces needed by other modules | RP2040 on a Raspberry Pico board | CU |
|Rotary encoders| Button interfaces with human. They can forward human commands to central unit: turned direct, turned indirect or pushed (they possess a push button too.). This module also gathers legacy push-buttons of radio which will be re-used to select radio mode | GPIOs are required, 3 per Rotary Encoder | Iduino Rotary Encoders |RE |
|Bluetooth Chip| Provide a Bluetooth connection to audio stream sources, retrieve information about connected device and track metadata and provide an audio differential output | GPIOs and UART are needed to control this device | RN52 | BT|
|FM demodulator | Can demodulate FM radiowaves and provide the audio output. It is also able to decode RDS metadata| I2C and UART are required to control this device | si470x| FM|
|e-paper screen| Is used to print all info, status, any kind of output judged as relevant for human user. Like track metadata, FM status or BT status | SPI is required | Waveshare e-paper 5.83" | EP |
|Audio Stage| Drives the 8 Ohms loudspeaker and switches between FM and BT modules depending on what CU orders | One GPIO is required | TPA6112A, PMA8302, TS5A2234 | AS |
|Power Management| Provide power supply to system. Is able to generate a 3.3V from either a 5V power supply, or a 12V power supply (battery)| USB plug and wires | LM317, NXE2S1205MC DC-DC converter |PM|
|Auxiliary Connector| Most of modules have debug interfaces as well as additional interfaces available. They are all gathered here, for future purpose if needed. SWD and UART of RP2040 are available here for debug| SPI, UART, GPIOs | Simple headers | AUX |

**Note:** This system view is also deploymen-view like. Therefore, no deployment view will be described in further documentation.

## Software architecture structure view

The software structural architecture is quite straigth-forward, as it doesn't have any particular complexity. Therefore, a simple layer architecture is quite enough:  

![Software architecture](./ARC42_architectureSoftware.svg "Software architecture")

Modules Pico SDK and RP2040 Hardware are actually "out of project" as they are already made, external modules. They are part of the structural architecture, but no work is to be done here.

### Detailled description of modules:

|Module| Desription| Software directory |
|:-:|:-|:-|
|Application (main)| main module, (starts with main.c in program). Describes the wished radio behavior and uses the other modules beneath it to realize it | [Software/code](../03_Development/Software/code/) |
|re_Buttons| Driver for rotary encoders, it provides handles to whom want to be notified if a rotary encoder was turn, in which direction it was turned and if push button was triggered. For legacy buttons, as only a direct GPIO state is enough, no code was added for them. They will be monitored directly by the Application (main module) | [Software/code/re_Buttons](../03_Development/Software/code/re_Buttons/) |
|bt_rn52| Driver for Bluetooth chip Microchip's RN52. It uses UART and GPIOs to configure the RN52 chip, send command and read results. I2S inteface is not used. Audio stream is directly outputed to differential output | [Software/code/bt_rn52](../03_Development/Software/code/bt_rn52/)|
|fm_si470x| Driver of si470x chip, which provides FM demodulation. It uses I2C and GPIOs interfaces to configure, control and retrieve RDS data from Si470x chip |[Software/code/fm_si470x](../03_Development/Software/code/fm_si470x/)|
|rdsDecoder| Provide tools to decode RDS frames notified by si470x. This module is actually only used by fm_si470x module and may be integrated to it |[Software/code/rdsDecoder](../03_Development/Software/code/rdsDecoder/)|
|hmi_ePaper| Driver of Waveshare e-paper. It mainly uses already provided driver from waveshare, which was a little bit reconfigured and cleaned for lightness and simplicity |[Software/code/hmi_ePaper](../03_Development/Software/code/hmi_ePaper/)|

**Note:**  
The whole code is written in C and uses CMake for compilation.

## Software architecture runtime view

This chapter describes, either with sequence, process or state machine UML diagrams, module's behavior as well as application's usecase.


### Application usecases

Rotary encoders allow infinte turn in direct or indirect rotating direction. Two rotary encoders are used for functional requirements described in [Intro and goals](../00_Requirements_Inputs/ARC42_IntroAndGoals.md).  
Depending on the mode (Bluetooth stream or FM stream) both of those rotary encoders enters a different behavior.  
The whole application runs as a state machine, depending on which mode is active (audio stream or FM demodulation). It is a simple state machine switching between mode BT (Bluetooth) and FM (FM demodulator). How user interacts with radio and how radio reacts is described below:  

#### Buttons and rotary encoders

**[Usecase 1 - standard audio stream control](../00_Requirements_Inputs/ARC42_IntroAndGoals.md)**  
**[Usecase 4 - Seek FM station](../00_Requirements_Inputs/ARC42_IntroAndGoals.md)**  

|Rotary encoder| Mode | Direct rotation | Indirect rotation | Push button |
|:-:|:-:|:-:|:-:|:-:|
|RE_1| Bluetooth | Volume- | Volume+ | Mute/Unmute |
|RE_1|FM demodulation| Same | Same | Same |
|RE_2| Bluetooth | Previous track | Next track | Play/Pause |
|RE_2|FM demodulation| Previous Station | Next Station | Select favorite ([behavior here](#select-favorite-fm-channel)) |

**[Usecase 2 - connect modern device](../00_Requirements_Inputs/ARC42_IntroAndGoals.md)**  
Phillips radio has many push buttons on top of it. Some are push/hold/exclude, some are just push/hold and some are just push.

Button | Place | Style | Role |
|:-:|:-:|:-:|:-:|
|X    | Far left | Push/Hold/exclude | Select bluetooth streaming |
|IX   | | Push/Hold/exclude| Select FM demodulation|
|VIII | | Push/Hold/exclude| none |
|VII  | | Push/Hold/exclude| none |
|VI   | | Push/Hold/exclude| none |
|V    | | Push/Hold/exclude| none |
|IV   | | Push/Hold/exclude| none |
|III  | | Push/Hold| none |
|II   | | Push/Hold| none |
|I    | Far right | Push Hold| On/Off |

**[Usecase 3 - Select and save FM station](../00_Requirements_Inputs/ARC42_IntroAndGoals.md)**  
Rotary encoder 2 has a specificity when entering "Select Favorite". There are many different stations and be able to select a favorite is nice. Saving a favorite should be possible, as station differs from place to place.

When clicking PushButton of RE_2 in FM mode, the following behavior appears:  
![Behavior select and save FM station](./Architecture_Behavioral_Diagrams/ARC42_behavioral_selectFMStation.svg)


### FM behavioral

@todo - also may need a bugfix when seeking/tuning and so on

### BT behavioral

No particular behavioral, look at doxygen API documentation

### RE behavioral

Rotary encoders work as state machines. As soon as one of the input of rotary encoder is triggered (GPIO_A or GPIO_B), they will enter a "wait for the other input to determine direction":  

![Rotary encoder behavior](./Architecture_Behavioral_Diagrams/ARC42_behavioral_RotaryEncoders.svg)

### EP behavioral


[< prev (Context and scope)](../01_Analysis/ARC42_ContextAndScope.md) - [next > (Decision, Risks and Technical debt)](../02_Architecture_Modelisation/ARC42_RiskDebtDecisions.md)

## Hardware architecture view

This section provides a detailled view of circuit schematic and PCB design. There is no behavioral described, as it may be found in the corresponding datasheet of components and will only be a re-write of what already exists.

### Schematic

Schematic is divided into thre sheets: a main sheet with modules. An audio amplifiying stage sheet and a connectors to legacy push buttons sheet.

#### Main schematic
![Main schematic](../06_Doc/pictures/Board_Bluetooth_radio_platform.svg)

#### Audio amplification stage schematic
![Audio stage](../06_Doc/pictures/Board_Bluetooth_radio_platform-audio_amp_stage.svg)

#### Legacy push buttons connectors schematic
![Legacy push buttons](../06_Doc/pictures/Board_Bluetooth_radio_platform-BtRadio_phillipsConnectors.svg)


### PCB
After routing, PCB has the following face:  

![PCB 2D view](../06_Doc/pictures/Board_Bluetooth_radio_platform_2Dview.png)

![PCB 3D view](../06_Doc/pictures/Board_Bluetooth_radio_platform_3Dview.png)

@todo: holes, form and connectors placement dimension measure list.