# This project is still under construction.

Do not attempt to reproduce it with the given files at the current state.
Documentation is still lacking, and the 3D models are missing.

# Chronodecahedron

 [![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0.html)
 [![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc/4.0/)


https://esphome.io/ powered physical time-tracking dodecahedron.

## Licensing

 - [AGPL-3.0 or later](./LICENSE.AGPL-3.0-or-later) applies to the code and configuration `configuration.yaml`
 - [CC-BY-NC-4.0](./LICENSE.CC-BY-NC-4.0) applies to the 3D-model, description and other support material

## What is this?

This device is a physical time-tracking device to easily track how long you work on what.

It consists of a 3d-printed dodecahedron (12 sided platonic solid) and uses an 
accelerometer to detect which side faces up.

By using [esphome](https://esphome.io) as a platform, it is then able to expose 
the detected side as a sensor over MQTT or other protocols.

## What do I need?

### Tools

 - Soldering Iron
 - Desoldering tools
   - Pump, wick or whatever you are able to use
 - 3D Printer
 
### Parts

| Name                             | Pcs | Note                              |
|----------------------------------|-----|-----------------------------------|
| **Prints**                       |     |                                   |
| Printed `hull.stl`               | 1   | Use your preferred color+material |
| Printed `base.stl`               | 1   |                                   |
| **Mechanical**                   |     |                                   |
| M3 threaded inserts              | 2   |                                   |
| M3x8 screws                      | 2   |                                   |
| M2 threaded inserts              | 2   |                                   |
| M2x8 screws                      | 2   |                                   |
| Double-sided tape                |     |                                   |
| **Electrical**                   |     |                                   |
| Lolin32 Lite                     | 1   |                                   |
| GY-521 (MPU-6050 Accelerometer)  | 1   |                                   |
| Resistor 100kΩ THT               | 2   |                                   |
| Li-Ion battery (with protection) | 1   |                                   |
| Jumper wire                      |     | `GND`/`INT` conection             |
| 6x2.54mm pin header              | 1   | Direct connection of the GY-521   |
| Solder                           |     |                                   |

## Assembly

### 0) Sourcing

 - Source the parts on your own
 - Print the parts with whatever material you prefer

### 1) Threaded Inserts

 - Melt the two M3 inserts into the holes of the printed `hull.stl`
 - Melt the two M2 inserts into the two blind holes of the printed `base.stl`

### 2) Electronics

 - Desolder the JST-PH header for the battery (We will connect the battery directly)
 - Desolder the LED on the GY-521 (white component next to the x-axis silkscreen)
 - Stick both 100kΩ resistor with one lead through `GPIO33` and make a solder connection
 - Solder the other lead of one of the 100kΩ resistors to the `+`-pad of the battery connector
 - Stick the other lead of the second resistor through `GND`, **but don't connect it yet**
 - Put a piece of jumper wire (~6cm) through `GND`, now solder both the resistor and wire to GND
 - Solder another short piece of jumper wire (~2cm) on `INT`of the GY-521
 - Break away one of the pins of the pin header (you will have one single pin and a row of 5)
 - Solder the single pin on VCC of the GY-521, the remaining row on the pins `SCL` to `ADO`
 - Solder the other end of the long jumper wire to `GND` of the GY-521
 - Stick the GY-521 into `3V` to `GPIO17` of the Lolin32 Lite (This will leave `GPIO22` and `GPIO16` unconnected)
   - Make sure that it's relatively parallel to the Lolin32 Lite
 - Solder the other end of the short jumper wire to `GPIO13` of the Lolin32 Lite
 - Cut the pins on the back of the lolin32 flush
 - Remove the `-`-wire from the battery connector
 - Solder it to the `-`-pad of the lolin32 lite battery connector (closer to reset button)
 - Remove the `+`-wire from the battery connector
 - Solder it to the `+`-pad of the lolin32 lite battery connector (closer to `GND`-pin)
   - Make sure that the connection to the resistor stays intact

### 3) Firmware

 - Modify the `yml`-files in the `esphome` directory to your needs (especially `secrets.yml`)
 - Connect the Lolin32
 - Run `esphome run chrono.yml` in that directory
 - Check that it works as expected (Side detection, battery level, battery charging)
   - The battery level might require fine-tuning. Measure the actual battery voltage level and compare it to the raw reported one.
 
### 4) Assembly

 - Use the M2 screws to attach the Lolin32 to the `base.stl`
 - Use the double-sided tape to attach the Li-Ion battery to the inside of the `hull.stl`
 - Close the `base.stl` and the `hull.stl` and connect them with the M3 screws.
