# Graphite Flight Data Recorder
Graphite is an ESP32-based WiFi-configurable flight data recorder for high powered rocketry projects.

It's designed with the philosophy of "black box" flight recorders—meant to be a closed system; reliant on very little (if any) integration with other parts of the vehicle or avionics suite. The current version of the project will rely on external battery power, however
future versions of this project are planned to be fully self-contained, with their own internal battery (1S LiPo), and optional interconnects to log data from (or interact with) other components within the vehicle (such as flight computers and altimeters that support the [OpenLog](https://github.com/sparkfun/OpenLog) standard, servos, external sensors).

Flight logs are stored on an internal micro SD card and can be accessed via the wifi interface (or by removing the SD card).


# Getting Started
This code is still in its early stages *(read: mostly incomprehensible and sparsely commented).* With that in mind, you're more than welcome to pick through it, learn how things work, and discuss in the HPRC Slack.

### Setting up your dev environment 
This project is built very specifically for the [Seeed XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) using PlatformIO. Make sure you've installed and set up the following things before cloning this repo to your system:
1. [Microsoft VSCode](https://code.visualstudio.com/download)
2. [GIT](https://git-scm.com/download) and optionally the GIT Source Control functionality [within VSCode](https://www.youtube.com/watch?v=i_23KUAEtUM)
3. [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) extension vor VSCode
4. Optionally, grab the [Teleplot](https://marketplace.visualstudio.com/items?itemName=alexnesnes.teleplot) extension for VSCode *(for easily charting sensor / debug data sent over serial)*
5. [Espressif 32](https://registry.platformio.org/platforms/platformio/espressif32) platform for PlatformIO


# Dev Roadmap
There are lots of //todo comments in the various files of this project with more details and things not mentioned here, but the following is the current dev roadmap.

### Completed Items
- ✅ Verify sensor functionality
  - ✅[ADXL377](https://learn.adafruit.com/adafruit-analog-accelerometer-breakouts) Analog high-g accelerometer 
    <br> NOTE: Calibration is very difficult for this fella. We're hopefully going to be replacing it with an I2C high-g accelerometer such as the [H3LIS200](https://www.dfrobot.com/product-2314.html) or [H3LIS331](https://www.adafruit.com/product/4627)
  - ✅[DPS310](https://learn.adafruit.com/adafruit-dps310-precision-barometric-pressure-sensor/overview) I2C Barometric pressure and temperature sensor
- Base functionality for reading and translating sensor data
  - ❌ ~~Filter ADXL377 x/y/z data and convert to g and m/s²~~ <br> Abandoning this for now pending switch to serial accelerometer *(because they're factory-calibrated and will simply give us g-force or m/s² values directly)* or construction of a proper high-g test apparatus (centrifuge?)
  - ✅ Filter DPS310 temp and pressure data and convert to altitude <br> See main.cpp for details on the methods used to calculate altitude. *(Tl;dr it's using a modified form of the Barometric formula)*
- ✅ Debug Framework
  - ✅ Specialized debugMsg() functions to replace Serial.print with additional functionality <br> (See WL_DebugUtils.h for details)
  - ✅ Print sensor data to serial in [Teleplot](https://marketplace.visualstudio.com/items?itemName=alexnesnes.teleplot)-compatible format

### Current Items
- ✅ Implement SPIFFS internal file system *(non-SD card file system)* for storing webpage data
- ✅ Implement WiFi AP functionality
- ✅ Implement Base WebServer.h functionality for serving webpages
  - ❗ server.serveStatic for each page to send SPIFFS files to client upon request
- ✅ Implement mDNS for logger access via .local domain name (easier than typing the IP into the browser address bar)
- HTML content, styling, scripting for placeholder webpages
  - ❗ Placeholder **status page** (doubles as the homepage when first connecting to the logger)
    - Current time w/ sync button
    - Checkbox to enable auto-refresh
    - Current sensor data (Altitude, temp, pressure, x/y/z acceleration)
    - Current core configuration data
    - Arm button w/ confirm prompt <br>
      Disallow arm if time not synced or any other errors present
  - ❗ Setup page
    - Entries for each configurable w/ inputs (radio, checkbox, dropdown, slider, textbox, etc.)
    - Apply button (applies each variable)
    - Apply and save button (applies each variable *and* saves it to nvs via Preferences)
    - Save config to SD button
  - ❗ Flight logs
  - ❗ Documentation page
- ❗ Store configuration data in non-volatile storage using ESP32 [Preferences](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html) library

### Next items
- Implement WebServer.h and webpage XML functionality 
  - Sending live data to client
    - Sensor data
    - Pre-populate all entries on config page with current config settings
  - Receiving instructions from client
    - Time sync button
    - Configuration data
      - Apply, apply and save buttons: client sends all config settings from page, esp applies them (and saves to nvs if applicable), trigger page refresh
      - Save config data to SD button: Create a new .txt file with all current config data flags
    - Arming
      - When armed: Remove all nav buttons to other pages and show armed state *(switch to displaying armed status page?)*, disallow all config changes, start launch detection code
- Implement SD card file system
  - Determine csv field format for logfile
    - Timestamp
    - Altimeter data (pressure, temp, altitude in m or ft or both?)
    - Accelerometer data (x,y,z in g or m/s² or both?)
    - Battery voltage
    - Other data? (OpenLog entries?)
    - Notes field (for logging special events like T0, apogee, ejection, etc.)
  - Create a new logfile and start logging data at background rate (very slow speed) when client arms rocket (detect via global armed status flag)
  - Terminate logfile if client disarms rocket
  - Switch to high-speed logging when launch detected flag is set
  - Add functionality to logs page
    - List flight log files
		- Download button next to each file
		- Delete button (w/ confirm prompt) next to each file
- Sensor fusion to set launch detected flag
  - Only execute checks if armed flag is set
  - Accelerometer data: if acceleration in any axis is greater than n
  - Altitude data: if altitude is greater than n <br> *OR if climbing at x rate for longer than n ms?*
  - *OpenLog data from Quasar?* <br> *(Must verify if Quasar serial uses 3.3V logic, else we'll need a high-speed level shifter)*
  - Write trigger reason to logfile
	- Write T0 timestamp to logfile
- React to launch detected flag
  - Shut down all wifi stuff (AP, webserver, mDNS)
- Make documentation webpage
  - Instructions for *everything*
- OpenLogger serial input (for Quasar data)
	-  Only if quasar output is 3.3V logic; not worth adding a level shifter for this
	- Simulate data stream w/ teensy for testing?

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
    - [ADXL375](https://www.adafruit.com/product/5374) *(not as good as the ST options above)*
- Automatic generation of "Graphite Readme.txt" file on SD card at startup if it's missing or empty (to ensure every SD card has the file, or in case it gets deleted)
  - Basic instructions on how to connect to the logger, wifi password recovery, github links, etc. 
- Battery level detection for *internal battery*
- Better webpage styling with *~top notch grafix~* and ~~dank memes~~
- Output framework
  - Detect flight events and trigger pre-configured outputs:
    - Servo control
    - Digital + PWM pin controls
    - *RGB???*
  
