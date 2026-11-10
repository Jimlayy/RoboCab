# Steering Module

## Functionality

The steering module can be used in both manual and autonomous mode depending on which UART data packets are sent.

UART data packets sent to the steering module has the following format:
| Order | Name | Value(s) | Description |
|:---:|:---:|:---:|:---:|
| 1 | Header | 0xAA | Identifier to let the parser know that a data transmission has begun |
| 2 | Length | 0...255 | How many payload bytes that will be sent in this packet |
| 3 | Message ID (first payload byte) | Check enum "MESSAGE_IDS" in definitions.h | Tells the parser who the data is meant for |
| 4 | Payload byte 2 | 0...255 | First actual value which will be assigned to some variable |
| 5 | Payload byte 3 | 0...255 | Second actual value in payload |
| . | . | 0...255 | . |
| . | . | 0...255 | . |
| N - 1 | Payload byte N | 0...255 | Last payload byte |
| N | Footer/Checksum | XOR of all payload bytes | Checksum for validating recieved data |


### Manual Mode

Manual mode is used when directly assigning values to the registers corresponding to the motor and servo. 

The format for a manual mode data **payload** looks like the following:

| Order | Type | Name | Value Range | Description |
|:---:|:---:|:---:|:---:|:---:|
| 1 | bool | Motor Direction | 0/1 | Turn motor forwards (1) or backwards (0)|
| 2 | bool | Motor Brake | 0/1 | Short circuits the motor to instantly brake |
| 3 | uint8_t | Motor Speed | 0...255 | The speed at which the motor will turn |
| 4-5 | uint16_t | Servo Pulse Width (high and low byte) | 1000...2000 | Which angle the servo will stand at (~1500 = middle) |

### Autonomous Mode

Autonomous mode is used when you want the car to drive by itself. The autonomous mode is run by a PD-loop. 

The format for a autonomous mode data **payload** looks like the following:

| Order | Type | Name | Value Range | Description |
|:---:|:---:|:---:|:---:|:---:|
| 1 | uint8_t | Delta T | 0...100 | Time since last PD-calculation (in milliseconds)|
| 2 | uint8_t | Sensor Mode | Check enum "SENSOR_MODES" in pwm.h | Tells the PD-controller which sensor the following data corresponds to |
| 3-4 | int16_t | Error | -32768...32768 | How "far away" we are from our wanted sensor reading |

**or**

| Order | Type | Name | Value Range | Description |
|:---:|:---:|:---:|:---:|:---:|
| 1 | uint8_t | Proportional constant | 0...255 | How intense the PD-controller should react to large errors|
| 2 | uint8_t | Derivative constant | 0.255 | How intense the PD-controller should react to a large difference in error between two PD-calculations |
