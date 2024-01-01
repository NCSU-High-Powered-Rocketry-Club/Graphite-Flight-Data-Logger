# Graphite Flight Data Recorder
Graphite is an ESP32-based WiFi-configurable flight data recorder for high powered rocketry projects.

It's designed with the philosophy of "black box" flight recorders—meant to be a closed system; reliant on very little (if any) integration with other parts of the vehicle or avionics suite. The current version of the project will rely on external battery power, however
future versions of this project are planned to be fully self-contained, with their own internal battery (1S LiPo), and optional interconnects to log data from (or interact with) other components within the vehicle (such as flight computers and altimeters that support the [OpenLog](https://github.com/sparkfun/OpenLog) standard, servos, external sensors).

Flight logs are stored on an internal micro SD card and can be accessed via the wifi interface (or by removing the SD card).

This project was started in Fall 2023 for the "Pencil Pusher" supersonic vehicle, a WolfWorks Experimental project from the [High Powered Rocketry Club](https://ncsurocketry.org/) at North Carolina State University

# Getting Started
This code is still in its early stages *(read: mostly incomprehensible and sparsely commented).* With that in mind, you're more than welcome to pick through it, learn how things work, and discuss in the HPRC Slack.

Note: I'm not currently accepting pull requests until after the next few club meetings in January to make sure everyone who want's to contribute is on the same page with our project goals.

### Setting up your dev environment 
This project is built very specifically for the [Seeed XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) using PlatformIO. Make sure you've installed and set up the following things before cloning this repo to your system:
1. [Microsoft VSCode](https://code.visualstudio.com/download)
2. [GIT](https://git-scm.com/download) and optionally the GIT Source Control functionality [within VSCode](https://www.youtube.com/watch?v=i_23KUAEtUM)
3. [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) extension vor VSCode
4. Optionally, grab the [Teleplot](https://marketplace.visualstudio.com/items?itemName=alexnesnes.teleplot) extension for VSCode *(for easily charting sensor / debug data sent over serial)* <br> You might also like the [Code Spell Checker](https://marketplace.visualstudio.com/items?itemName=streetsidesoftware.code-spell-checker) extension; it's spellcheck but for your code.
5. [Espressif 32](https://registry.platformio.org/platforms/platformio/espressif32) platform for PlatformIO
6. PlatformIO should automatically install all other dependencies when you build the project. 

Once that's done, check out the various [Tutorials](https://docs.platformio.org/en/latest/tutorials/index.html) for using PlatformIO if you haven't used it before. There are a few key differences between PIO and the Arduino IDE (like the project configuration file, [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html), which replaces the Tools menu settings from the Arduino IDE, helps manage your library dependencies, and more)


# Dev Roadmap
There are lots of `TODO` comments in the various files of this project with more details and things not mentioned here, but the following is the current development roadmap:

### Completed Items
- ✅ Verify sensor functionality
  - ✅[ADXL377](https://learn.adafruit.com/adafruit-analog-accelerometer-breakouts) Analog high-g accelerometer 
    <br> NOTE: Calibration is very difficult for this fella. We're hopefully going to be replacing it with an I2C high-g accelerometer such as the [H3LIS200](https://www.dfrobot.com/product-2314.html) or [H3LIS331](https://www.adafruit.com/product/4627)
  - ✅[DPS310](https://learn.adafruit.com/adafruit-dps310-precision-barometric-pressure-sensor/overview) I2C Precision barometric pressure and temperature sensor (altimeter)
- ✅Base functionality for reading and translating sensor data
  - ❌ ~~Filter ADXL377 x/y/z data and convert to g and m/s²~~ <br> Abandoning this for now pending switch to serial accelerometer *(because they're factory-calibrated and will simply give us g-force or m/s² values directly)* or construction of a proper high-g test apparatus (centrifuge?)
  - ✅ Filter DPS310 temp and pressure data and convert to altitude <br> See main.cpp for details on the methods used to calculate altitude. *(Tl;dr it's using a modified form of the [Barometric formula](https://en.wikipedia.org/wiki/Barometric_formula))*
- ✅ Debug Framework
  - ✅ Specialized debugMsg() functions to replace Serial.print with additional functionality <br> (See WL_DebugUtils.h for details)
  - ✅ Print sensor data to serial in [Teleplot](https://marketplace.visualstudio.com/items?itemName=alexnesnes.teleplot)-compatible format
- ✅ Implement [SPIFFS](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html) internal file system *(non-SD card file system)* for storing webpage data
- ✅ Implement WiFi AP functionality
  - Also added WiFi dev mode flag, if set to true WiFi will start in STA mode and connect to a pre-defined network (i.e. the same wifi network your PC is on)
- ✅ Implement Base WebServer.h functionality for serving webpages
  - ✅ server.serveStatic() and server.on() callbacks to stream SPIFFS files to client upon request <br>
    In some cases, client requests are denied or different items are returned depending on internal state flags (like armed status)
- ✅ Implement mDNS for logger access via .local domain name (easier than typing the IP into the browser address bar)

### Current Items

- ❗ Store configuration data in non-volatile storage using ESP32 [Preferences](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html) library
- HTML content, styling, scripting for core webpages
  - ❗ Placeholder **status pages** (doubles as the homepage when first connecting to the logger)
    - ✅ Current time w/ sync button (sends the current time from the client to the ESP32 to set the ESP32's internal RTC, since we can't use NTP)
    - ✅ Current sensor data (Altitude, temp, pressure, x/y/z acceleration)
    - ✅ Current primary configuration data (placeholders)
    - ✅ Arm button w/ confirm prompt <br>
        Disallow arm if time not synced, not enough space on SD (placeholder until SDFat implemented), or any other errors present (Future idea: detect non-nominal sensor conditions and prevent arming?)
    - ✅ Different status page when armed
    - Navigation buttons in header (for Setup, Flight Logs, Docs)
  - ❗ Setup page
    - Entries for each configurable w/ inputs (radio, checkbox, dropdown, slider, textbox, etc.)
    - Apply, apply and save buttons for each variable: client sends all config settings from page, esp applies them (and saves to nvs if applicable), trigger page refresh
    - Save config to SD button: generate a ExportedSetup_[timestamp].txt file on SD with all configuration key:values and a description of each (disallow if time not synced)
  - ❗ Flight logs page
  - ❗ Documentation page

### Next items
- Implement SD card file system ([SDFat](https://github.com/greiman/SdFat))
  - Finalize csv field format for log file
    - Timestamp (T+ mm:ss:ms *and/or* regular mm/dd/yyyy hh:mm:ss)
    - Altimeter data (pressure, temp, altitude in m or ft or both?)
    - Accelerometer data (x,y,z in g or m/s² or both?)
    - Battery voltage
    - Other data? (OpenLog entries?)
    - Notes field (for logging special events like T0, apogee, ejection, landing, etc.)
  - Create a new log file and start logging data at background rate (very slow speed) when client arms rocket (detect via global armed status flag) <br>
    Use "Flight log: " + Timestamp at moment started as logfile name
  - Terminate log file if client disarms rocket 
  - Switch to high-speed logging when launch detected flag is set
  - Terminate log file if SD Card is almost full (at any point, regardless of armed / in-flight state flags)
  - Add functionality to logs page
    - List flight log files (or just list all SD files)
		- Download button next to each file
		- Delete button (w/ confirm prompt) next to each file
- Add ADXL377 detection logic to setup()
  - We need some kind of verification that the sensor's alive and working normally; if it's not then we need to stop the program, else junk data from the analog pins might disrupt launch detection
- Sensor fusion for launch detection
  - Only execute checks if armed flag is set
  - Accelerometer data: if acceleration in any axis is greater than n
  - Altitude data: if altitude is greater than n <br> *OR if climbing at x rate for longer than n ms?*
  - *OpenLog data from Quasar?* <br> *(Must verify if Quasar serial uses 3.3V logic, else we'll need a high-speed level shifter)*
  - Set launch detected flag to true
  - Write trigger reason to log file
	- Write T0 timestamp to log file
- Sensor fusion for landing detection
    - Only execute checks if launched
    - Altimeter: if altitude reading is be low n for greater than m ms?
    - Accelerometer: *??? (not as simple as "if acceleration below n gs for greater than m ms, because acceleration will be low during descent)* 
    - Set landing detected flag
    - Write landing detected reason to log file
    - Write landing timestamp to log file
- React to launch detected flag
  - Shut down all wifi stuff (AP, webserver, mDNS)
  - Start logging at fast rate
- Logic to stop logging and shut down
  - Stop logging after n minute timeout or if SD card full: write reason to log file, close file, shut down ESP (enter ultra low power mode).
  - If landing detected flag is set: Switch to background logging rate, wait for n minutes, stop logging and close log file, shut down ESP (enter ultra low power mode)
- CSS stylesheet (used by all pages)
- Code refactor & cleanup 
  - Move extraneous functions / methods from main.cpp to their own files
  - Format all code and comments neatly
- Make documentation webpage
  - Instructions for *everything*
- *Sensor fusion for apogee and e-fuse activaton detection*
- *OpenLogger serial input (for Quasar data)*
	- *Only if quasar output is 3.3V logic; not worth adding a level shifter for this*
	- *Simulate data stream w/ teensy for testing?*
  - *Add config option to sync time from Quasar GPS NMEA frames instead of web server client's clock*

## Hardware Roadmap
- Diode protection (or full wave rectifier) on battery input connector
- 5V regulator w/ large storage caps to feed ESP32 VIN pin (placed after protection diodes)
- Battery level detection for external battery
  - Sense via voltage divider after battery connector rectifier but before 5V regulator
  - Record with other sensor data, display on status page
  - Cal value for low battery level
- Low profile header for ESP32 module (hot glue to prevent detachment in flight)
- Hot glue Sense board and micro SD card to main ESP32 board
- I2C routing for sensors
- Implement low battery shutdown
  - **DISABLE FOR JANUARY LAUNCH!** <br> We need to collect battery level data from the ejection charges firing to ensure the voltage drop from the e-matches won't trigger a low-battery false alarm
  - Activates if battery voltage drops below cal value for > n µs
  - Create final log entry with shutdown reason
  - Close current log file(s) if any are open
  - Shut down everything
  - Put ESP in ultra low power mode (only draws 7µA; best we can do without an external PMIC that'd kill power to the ESP completely)
- *Neopixel status LED?*
- 3D Printed black box case 
  - Holes for USBC and micro SD
  - Light pipes for LEDs
  - Pencil Pusher version without space for a battery, hole for battery connector instead

## Future Wishlist
- Switch to I2C high g accelerometer
  - Gets us back 4 pins for other stuff + remove calibration headache
  - Possible options: 
    - [H3LIS331DL](https://www.adafruit.com/product/4627)
    - [H3LIS200DL](https://www.dfrobot.com/product-2314.html)
    - [ADXL375](https://www.adafruit.com/product/5374) *(not as good as the LIS options above)*
- Move from [ESP32Time](https://github.com/fbiego/ESP32Time) library to internal RTC management methods within ESP framework <br>
  These aren't well documented for the Arduino-ESP32 core; ESP32Time uses them under the hood, so it was easier to use that and get things working quick and dirty
- Automatic generation of "Graphite Readme.txt" file on SD card at startup if it's missing or empty (to ensure every SD card has the file, or in case it gets deleted)
  - Basic instructions on how to connect to the logger, wifi password recovery, github links, etc. 
- Battery level detection for *internal battery*
- Better webpage styling with *~top notch grafix~* and ~~dank memes~~
- Output framework
  - Detect flight events and trigger pre-configured outputs:
    - Servo control
    - Digital + PWM pin controls
    - *RGB???*
  
# Appliccable Licenses and Libraries
The contents of this project (unless otherwise noted below) are distributed as a whole under the GNU General Public License 3.0. 
Please see the LICENSE file included in this project's repository for details

**This project created using the Arduino SDK within PlatformIO on Microsoft Visual Studio Code** <br>
  https://www.arduino.cc/ (GNU Lesser General Public License Version 2.1, see Arduino.h, https://github.com/arduino/arduino-ide/blob/main/LICENSE.txt) <br>
  https://platformio.org/ (Apache License Version 2.0, January 2004, https://github.com/platformio/platformio-core/blob/develop/LICENSE) <br>
  https://code.visualstudio.com/ (MIT License, https://github.com/microsoft/vscode/blob/main/LICENSE.txt)

**Arduino Core for the ESP32** <br>
  *(And it's associated built in libraries)* <br>
  https://github.com/espressif/arduino-esp32 (GNU Lesser General Public License Version 2.1) <br>
  Copyright (c) 2023, Espressif.

**ESP32Time Internal RTC Library** <br>
  https://github.com/fbiego/ESP32Time (MIT License)
  Copyright (c) 2021 Felix Biego

**Adafruit DPS310 Library** <br>
  https://github.com/adafruit/Adafruit_DPS310 (BSD License) <br>
  Copyright (c) 2019, Limor Fried for Adafruit Industries