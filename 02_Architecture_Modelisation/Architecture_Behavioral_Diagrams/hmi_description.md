# Project Bluetooth radio

## Human to Machines Interface
This page describes all interactions possible between human and bluetooth radio
This communication is done through:
 - Rotary encoders
 - Push switches of rotary encoders
 - Push buttons on the top of Phillips Hurricane radio
 - Screen (epaper)
 
## Rotary encoders

Rotary encoders allow infinte turn in direct or indirect rotating direction. They are located at bottom left part of radio, two of them.
Depending on the mode (Bluetooth stream or FM stream) both of those rotary encoders enters a different behavior.

Rotary encoders generally also offers a push button when pushing their knob. It allows to have for all rotary encoders, the capacity to send 3 different signals to a microcontroller:
 - Direct rotation
 - Indirect rotation
 - Button pushed

### Rotary encoder 1

Rotary encoder 1 is placed on most left part of the radio and is responsible for sound volume level.
Therefore, its behavior is the same for both modes.

| Mode | Direct rotation | Indirect rotation | Push button |
|:-:|:-:|:-:|:-:|
| Bluetooth | Volume- | Volume+ | Mute/Unmute |
|FM demodulation| Same | Same | Same |


### Rotary encoder 2

Rotary encoder 2 is placed at the right of RE_1 and is specific to the mode.

| Mode | Direct rotation | Indirect rotation | Push button |
|:-:|:-:|:-:|:-:|
| Bluetooth | Previous track | Next track | Play/Pause |
|FM demodulation| Previous Station | Next Station | Select favorite |

#### Select Favorite specific mode
Rotary encoder 2 has a specificity when entering "Select Favorite". There are many different stations and be able to select a favorite is nice.
Saving a favorite should be possible, as station differs from place to place.

When clicking PushButton of RE_2 in FM mode, the following behavior appears:

[Show list of favorites]
[Set cursor to first index of favorite list]

IF(RE_2 turned indirectly)
    [Move cursor to next saved favorite]
    IF(index > MAX_INDEX)
        [index = 0]
IF(RE_2 turned directly)
    [Move cursor to previous favorite]
    IF(index < 0)
        [index = MAX_INDEX]
    
IF(PB pushed)
    [Select the favorite station on which cursor is]
    [Quit favorite selection mode]
    
IF(cursor is on "save actual station")
    IF(RE_2 turned indirectly)
        [Move cursor to next saved favorite]
        IF(index > MAX_INDEX)
            [index = 0]
    IF(RE_2 turned directly)
        [Move cursor to previous favorite]
        IF(index < 0)
            [index = MAX_INDEX]
    IF(PB pushed)
        [Save actual station on which cursor is]
        [Quit favorite selection mode]
    
    
## Phillips radio push button

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


