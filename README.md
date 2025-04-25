# renogy-bt-esphome
ESPHome implementation to pull data from BT-enabled Renogy devices

## Setup
- renogy_batteries.yaml is configured to connect to a single Renogy BT device (currently tested on a BT-2 connected to five batteries in a daisy-chained configuration.) There are TODOs in the yaml that indicate places where updates should be made, or at least pertinent information for the user is available.

  - renogy_battery_utilities.h contains methods to do the payload creation and response parsing for communicating with the BT device. This file shouldn't require any editing in order to get things running.


- renogy_rover.yaml is configured to connect to a Renogy BT device (tested on a BT-2.) There are TODOs in the yaml that indicate place where updates should be made.

  - renogy_rover_utilities.h contains methods to do the payload creation and response parsing for communicating with the BT device. This file shouldn't require any editing in order to get things running.

### Getting started with ESPHome
If you don't yet have any experience with ESPHome, I recommend looking here for guidance: https://esphome.io/guides/getting_started_command_line
I use the command line to compile and upload my configurations to my ESP32 board(s) and therefore don't have any experience with the ESPHome web interface (glancing through the docs, I don't know how to use header files like `renogy_battery_utilities.h` that are used in this project, so the CLI is probably your best bet.)
Here are instructions to install ESPHome manually (which is what I did.) https://esphome.io/guides/installing_esphome

Once that is done, you can run commands such as these from the root of this project:
`esphome compile renogy_batteries.yaml` # (compiles without uploading, so you can test your changes without needing a board connected)
`esphome run renogy_batteries.yaml` # (compiles and uploads to a connected board)

### Disclaimer

This is not an official library endorsed by the device manufacturer. Renogy and all other trademarks in this repo are the property of their respective owners and their use herein does not imply any sponsorship or endorsement.

## References
 - [Olen/solar-monitor](https://github.com/Olen/solar-monitor)
 - [corbinbs/solarshed](https://github.com/corbinbs/solarshed)
 - [Rover 20A/40A Charge Controller—MODBUS Protocol](https://github.com/cyrils/renogy-bt/files/12787920/ROVER.MODBUS.pdf)
 - [Lithium Iron Battery BMS Modbus Protocol V1.7](https://github.com/cyrils/renogy-bt/files/12444500/Lithium.Iron.Battery.BMS.Modbus.Protocol.V1.7.zh-CN.en.1.pdf)
 - [The original Renogy BT project](https://github.com/cyrils/renogy-bt) - This current project is effectively a port of cyrils' work to make it run on ESPHome. None of this would be possible without the groundwork that was laid there.
