# CanopyLights

## Installation

There are two options for switching power to the LEDs - using a door switch to control the internal relay, or an external relay that you supply.

### Internal relay method

This only works for switched-ground circuits, (i.e. one side of the door switch connects directly to ground).

With this method, the unit receives a constant 12V. The LEDs will power on when door switch closes and the DOOR terminal is shorted to the vehicle's ground.

Please note that the unit will draw ~20mA at idle using this method.

1. Connect the **12V IN** terminal via a 5A fuse to 12V (+).
2. Connect the **DOOR** terminal to the desired door switch
3. Follow the common steps below

### External relay method

With this method, the module powers up (and the LEDs turn on) whenever the external relay supplies 12V. It cannot draw any current when the external relay is off.

1. Connect the **12V IN** terminal via a 5A fuse to the switched 12V output of your external relay
2. Short the **DOOR** terminal to one of the **GND** terminals
3. Follow the common steps below

### Common steps

* Connect one of the **GND** terminals to the vehicle's ground (-).
* Connect the **LED +** terminal to the LED strip's +12V line
* Connect the **DATA** terminal to the LED strip's D0 and B0 lines
* Connect a **GND** terminal to the LED strip's GND line
* Connect the **BUTT** terminal to one side of the switch you want to use for changing colour/brightness, and connect the other side of the switch to GND
