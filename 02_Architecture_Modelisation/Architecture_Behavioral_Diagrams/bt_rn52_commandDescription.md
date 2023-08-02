# RN52 Bluetooth communication reference and info

RN52 Bluetooth chip may be controlled and data from it get/set with a set of AT commands synthax  

For each command:
When a command is issued, RN52 module responds with an encoded 2-byte 
stream of ASCII hex data terminated by the `\r\n`

## RN52 available commands for version V1.16
### SET COMMANDS 

|AT cmd| format | Description |
|:-:|:-|:-|
|`S-`|<text>  | Serialized Name|
|`S|`|<hex16> | Audio Settings|
|`S^`|<dec>   | Auto Power Off timer|
|`S%`|<hex16> | Extended Features|
|`SA`|<dec>   | Authentication Mode|
|`SC`|<hex24> | Class of Device|
|`SD`|<hex8>  | Discovery Mask|
|`SF`|1       | Factory Default|
|`SK`|<hex8>  | Connection Mask|
|`SM`|<hex32> | Microphone Gain|
|`SL`|<hex32> | Line Input Gain|
|`SN`|<text>  | Name|
|`SP`|<text>  | Pin Code|
|`SS`|<hex8>  | Speaker Level|
|`ST`|<hex8>  | Tone Level|
|`SU`|<hex8>  | UART Baudrate|
|`STA`|<dec>  | Connection Delay(ms)|
|`STP`|<dec>  | Pairing Timeout(s)|

### ACTION COMMANDS
|AT cmd| format | Description |
|:-:|:-|:-|
|`+`|         |Disable,Enable Local Echo|
|`@`|<0,1>    |Disable,Enable Discovery|
|`#`|<0,1>    |Accept,Reject Pairing Passkey|
|`$`|         |Enter DFU Mode|
|`D`|         |Dump Configuration|
|`K`|<hex8>   |Disconnect|
|`M`|<0,1>    |Mute Active Call|
|`Q`|         |Connection Status|
|`R`|1        |Reboot|
|`U`|         |Reset Paired Device List(PDL)|
|`V`|         |Firmare Version|

### AVRCP COMMANDS
|AT cmd| Description |
|:-:|:-|
|`AV+` | Volume Up|
|`AV-` | Volume Down|
|`AT+` | Next Track|
|`AT-` | Previous Track|
|`AP`  | Play/Pause|
|`AD`  | Track Metadata|

### CONNECTION COMMANDS
|AT cmd| format | Description |
|:-:|:-|:-|
|`A`|<text> |Initiate Voice Call|
|`AR`|      |Redial Last Dialed Number|
|`B`|       |Reconnect Last Device|
|`C`|       |Accept Incoming Call|
|`E`|       |Drop/Reject Active Or Incoming Call|
|`F`|       |Release All Held Calls|
|`J`|       |Accept Waiting Calls And Release Active Calls|
|`L`|       |Accept Waiting Calls And Hold Active Calls|
|`N`|       |Add Held Call|
|`O`|       |Connect Two Calls And Disconnect The Subscriber|
|`T`|       |Caller ID Information|
|`P`|       |Initiate Voice Recognition|
|`X`|<0,1>  |Transfer Call Between HF And AG|

Q results:
0 iAP wireless active connection to remote device
1 SPP active connection to remote device
2 A2DP active connection to remote device
3 HFP/HSP active connection to remote device
4 Caller ID notification event from audio gateway
5 Track change event notification


## GPIOs needed for UART communication 
GPIO9
- drive GPIO9 low for CMD mode
- drive GPIO9 high for DATA mode
-> we use CMD mode only
GPIO4:  Reset to factory settings
GPIO7:  Driving this pin low sets the UART baud rate to 9,600 bps. By default the pin is high and the UART has a baud rate of 115200 bps
GPIO2:  Toggles from high to low for 100 ms to indicate that the module’s state has changed

## UART communication 
- 115,200 Kbps
- 8 bits
- Parity: none
- Flow control: enabled (but seems to work better without)

