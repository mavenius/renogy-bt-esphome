# renogy-bt-esphome
ESPHome implementation to pull data from BT-enabled Renogy devices

## Setup
renogy_batteries.yaml is configured to connect to a single Renogy BT device (currently tested on a BT-2 connected to three batteries in a daisy-chained configuration.) There are todos in the yaml that indicate places where updates should be made, or at least pertinent information for the user is available.

renogy_utilities.h contains methods that do the heavy-lifting regarding prepping requests to be sent to the BT device and parsing the response.

### Disclaimer

This is not an official library endorsed by the device manufacturer. Renogy and all other trademarks in this repo are the property of their respective owners and their use herein does not imply any sponsorship or endorsement.

## References
 - [Olen/solar-monitor](https://github.com/Olen/solar-monitor)
 - [corbinbs/solarshed](https://github.com/corbinbs/solarshed)
 - [Rover 20A/40A Charge Controller—MODBUS Protocol](https://github.com/cyrils/renogy-bt/files/12787920/ROVER.MODBUS.pdf)
 - [Lithium Iron Battery BMS Modbus Protocol V1.7](https://github.com/cyrils/renogy-bt/files/12444500/Lithium.Iron.Battery.BMS.Modbus.Protocol.V1.7.zh-CN.en.1.pdf)
 - [The original Renogy BT project](https://github.com/cyrils/renogy-bt)