# RN52 Bluetooth module command reference

## Intro

todo

## Things to know / short info

For each command:
When you issue this command, the module responds with an encoded 2-byte stream of ASCII hex data terminated by the \r\n


### GPIO9
- drive GPIO9 low for CMD mode
- drive GPIO9 high for DATA mode
-> we use CMD mode only

### GPIO2
Toggles from high to low for 100 ms to indicate that the module’s state has changed
Use AT command "Q" to know what happened

### GPIO3
For device firmware update purpose only: so unused as device not supported by Microchip anymore

### GPIO4
Reset to factory settings

### GPIO6
May be configured/programmed in firmware like a GPIO
-> unused

### GPIO7
Driving this pin low sets the UART baud rate to 9,600 bps. By default the pin is high and the UART has a baud rate of 115200 bps

## UART communication:
- 115,200 Kbps
- 8 bits
- Parity: none
- Flow control: enabled (but seems to work better without)


## Commands
Many commands are available, many (80%) are for device configuration and not "common" runtime mode.
Only those for UserExperience are listed here and get their corresponding function

|Command | Command description | AT synthax| example |
|:-:|:-:|:-:|:-:|
| sets speaker gain levels, from 00 to 0F | SS,<8-bit hex value> | SS,0A |


