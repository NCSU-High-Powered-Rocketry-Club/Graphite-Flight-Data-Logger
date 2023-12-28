/*
Flight Data Logger Project - Version Zero
For 2023-2024 "Pencil Pusher" Mach 1+ Vehicle
High Powered Rocketry Club, WolfWorks Team
North Carolina State University
https://github.com/WillsThingsNC/NCSU-HPRC-Graphite 


Core codebase written by Willard Sheets
  (Will's Lab) wbsheets@ncsu.edu / will@willsthings.com

Additional contributions by NCSU HPRC Team Members:
  Your name here

Hardware used:
  Seeed XIAO ESP32S3 Sense (with Sense board attached, camera + ribbon cable removed)
  Adafruit DPS310 barometric altimeter (pressure and temp)
  Adafruit ADXL377 3 axis high-g analog accelerometer (sweet jesus I hate this bastard)
  PCB with peripheral interconnects, power management, etc.

Applicable Licenses / Libraries:
  The contents of this project (unless otherwise noted below) are distributed as a whole under the GNU General Public License 3.0. 
    Please see the LICENSE file included in this project's repository for details

  This project created using the Arduino SDK within PlatformIO on Microsoft Visual Studio Code
    https://www.arduino.cc/ (GNU Lesser General Public License Version 2.1, see Arduino.h, https://github.com/arduino/arduino-ide/blob/main/LICENSE.txt)
    https://platformio.org/ (Apache License Version 2.0, January 2004, https://github.com/platformio/platformio-core/blob/develop/LICENSE)
    https://code.visualstudio.com/ (MIT License, https://github.com/microsoft/vscode/blob/main/LICENSE.txt)

  Adafruit DPS310 Library (BSD License)
    https://github.com/adafruit/Adafruit_DPS310
    Copyright (c) 2019, Limor Fried for Adafruit Industries

  Preferences.h Library for Arduino-ESP32 (Apache License 2.0)
    https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html
    https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h
    Copyright (c) 2015-2016 Espressif Systems (Shanghai) PTE LTD

  Wire.h TWI/I2C Library for Arduino (GNU Lesser General Public License version 2.1)
    https://github.com/esp8266/Arduino/tree/master/libraries/Wire
    Copyright (c) 2006 Nicholas Zambetti

*/

// Main includes --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  #include <Arduino.h>
  // #include "SPIFFS.h"  // Not used, switching to Preferences instead
  #include <Preferences.h> // For storing persistent configuration data in ESP32's NVS partition
  #include <Adafruit_DPS310.h>
  #include <WL_DebugUtils.h> // For debugMsg() functions (Serial.print(ln) with added functionality)

// Debug Mode -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  /* Note: 
  Additional debug information can be printed to serial by changing -DCORE_DEBUG_LEVEL=n in platformio.ini
    Options: 0=None, 1=Warn, 2=Info, 3=Debug, 4=Verbose
  Program debug message prefixes:
    [CRITICAL]  - Events that impact the base functionality of the device
    [ERROR]     - Errors that are not being handled gracefully
    [WARN]      - Errors that are being handled gracefully
    [INIT]      - Startup events
    [EVENT]     - General event logging
    [DATA]      - Data output (in verbose mode), followed by multiple lines (one for each value)
                  >[value]: [data] - format is used for Teleplot (VSCode plugin)
  */
  int debugMode = 2; // 0 = off, 1 = General, 2 = Verbose

// IO Defines -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  #define p_xAccel A0         // Accelerometer X analog pin
  #define p_yAccel A1         // Accelerometer Y analog pin
  #define p_zAccel A2         // Accelerometer Z analog pin
  #define p_testAccel D5      // Accelerometer self test pin
  #define p_SDA D3            // I2C Data pin (used by DPS310)
  #define p_SCL D4
  #define io_DPS310Address 0x77
  #define io_USBSerialSpeed pio_monitor_speed

// Instantiate Classes --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Adafruit_DPS310 dps;

// Global Variables -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  /* Note: 
      Default values are initialized here but may be overwritten in setup. 
      Persistent config variables are loaded from the nvs (non-volatile storage) partition in setup() via the Preferences library
  */
  // WiFi

  // Logging
  unsigned long io_StatLEDTimer;      // millis() timer for blinking the status LED
  bool io_StatLEDState = 1;           // Status LED state
  unsigned long io_logQuickTimer = 0;   // millis() timer for logging fast data to the SD card
  unsigned long io_accelSampleTimer = 0;// millis() timer for logging accelerometer samples
  unsigned long io_altSampleTimer = 0;  // millis() timer for logging altimeter samples
  #define io_logQuickTime 20          // How many ms to wait between logging fast data (accelerometer and altitude data)
  #define io_logSlowTime 100          // How log to wait between logging slow data (Nothing yet...)
  //Note: io_accelSamples * io_accelSampleRate must be <= io_logQuickTime to ensure enough samples are collected prior to averaging and logging
  #define io_accelSamples 4           // How many ADXL377 samples to average into each log entry (note: max safe sample rate is 300Hz, or once every ~3ms)
  #define io_accelSampleRate 5        // How many ms to wait before taking an accelerometer sample
  uint8_t io_accelCurrentSample = 0;  // Current accelerometer sample, used for storing successive samples to arrays
  //Note io_altSamples * io_altSampleRate must be <= io_logQuickTime to ensure enough samples are collected prior to averaging and logging
  #define io_altSamples 4             // How many DPS310 samples to average into each log entry (note: max safe sample rate is 300Hz, or once every ~3ms)
  #define io_altSampleRate 5          // How many ms to wait before taking an altimeter sample
  uint8_t io_altCurrentSample = 0;    // Current altimeter sample, used for storing successive samples to arrays

  // ADXL377
  bool cal_accelCalMode = 0, cal_accelCalStarted = 0;  // Used by accelerometer calibration routine
  int cal_zeroXAccel = 1984;  // X Zero point for accelerometer (value of analogRead when g force = 0)
  int cal_p1gXAccell = 1992;  // X Raw value at +1g
  int cal_n1gXAccell = 1975;  // X Raw value at -1g
  int cal_zeroYAccel = 1984;  // Y Zero point for accelerometer (value of analogRead when g force = 0)
  int cal_p1gYAccell = 1992;  // Y Raw value at +1g
  int cal_n1gYAccell = 1975;  // Y Raw value at -1g
  int cal_zeroZAccel = 1992;  // Z Zero point for accelerometer (value of analogRead when g force = 0)
  int cal_p1gZAccell = 2005;  // Z Raw value at +1g
  int cal_n1gZAccell = 1978;  // Z Raw value at -1g
  double cal_xAccelCoef = 0.03; // X Accelerometer raw to g coefficient (raw value * coef = g value)
  double cal_yAccelCoef = 0.03; // Y Accelerometer raw to g coefficient 
  double cal_zAccelCoef = 0.029; // Z Accelerometer raw to g coefficient 
  float dat_xAccelRaw, dat_yAccelRaw, dat_zAccelRaw;  // Current measured acceleration
  int dat_xAccelSamples[io_accelSamples];   //Array to hold raw X acceleration samples
  int dat_yAccelSamples[io_accelSamples];   //Array to hold raw Y acceleration samples
  int dat_zAccelSamples[io_accelSamples];   //Array to hold raw Z acceleration samples
  double dat_xAccelG, dat_yAccelG, dat_zAccelG;  // Calculated g force
  double dat_xAccelMs2, dat_yAccelMs2, dat_zAccelMs2;  // Calculated m/s^2 force

  unsigned long cal_accelCalTimer;  // Tracks how long it's been since calibration mode started
  unsigned long cal_accelCalTimeout = 60000;  // How long to wait (ms) before ending calibration mod

  // DPS310
  float dat_tempC;                      // Current measured temperature (C)
  float dat_tempF;                      // Current measured temperature (F)
  float dat_tempK;                      // Current measured temperature (K)
  float dat_pressPa;                    // Current measured Pressure (Pa)
  float dat_altMBaro;                   // Current calculated barometric altitude (m)
  float dat_altFtBaro;                  // Current calculated barometric altitude (ft)
  float dat_tempCSamples[io_altSamples];    // Array to hold temperature (C) samples
  float dat_tempFSamples[io_altSamples];    // Array to hold temperature (F) samples
  float dat_tempKSamples[io_altSamples];    // Array to hold temperature (K) samples
  float dat_pressPaSamples[io_altSamples];  // Array to hold Pressure (Pa) samples
  float dat_altMBaroSamples[io_altSamples]; // Array to hold calculated barometric altitude (m) samples
  float dat_altFtBaroSamples[io_altSamples];// Array to hold calculated barometric altitude (ft) samples
  float cal_lapseRate = 0.0059;         // Temperature lapse rate used in barometric altitude calculation
  float cal_magicExp = 0.190266435664;  // Exponent from barometric formula used in altitude calculation
  float cal_pAtSea = 101325;            // Pressure (Pa) at sea level
  /*Altitude calculation info: 
    (I'm not a magician, don't ask me how this shit works)
    Formulas via https://physics.stackexchange.com/questions/333475/how-to-calculate-altitude-from-current-temperature-and-pressure and https://en.wikipedia.org/wiki/Barometric_formula 
    - Formula used to calculate altitude(m): (((cal_pAtSea/pressurePa)^cal_magicExp - 1) * tempK)/cal_lapseRate
    - cal_lapseRate: Standard temperature lapse rate from 0-36kft is 0.0065, however this can be tweaked some depending on logger location (I tested the code in the mountains so lapse 
      rate was slightly smaller).
    - cal_magiExp: RL/GM (universal gas constant * temperature lapse rate / gravitational accel * Air's Molar Mass), equal to 1/5.25578774055 (this *is* calculated using L=.0065)
    - TODO: Add calibration sliders / text boxes in the config menu to adjust magic exp and lapse rate (would be cool if you could enter the current known altitude and have the 
      program work backwards, actually)
  */

// map() but for floats
float mapf(float num, float fromLow, float fromHigh, float toLow, float toHigh)
{
	return (num - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}


// Setup ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  // Init pins
  analogReadResolution(12); // Switch to 12-bit analog read resolution for precise reads of analog inputs
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(p_xAccel,INPUT);
  pinMode(p_yAccel,INPUT);
  pinMode(p_zAccel,INPUT);
  
  pinMode(D7,INPUT_PULLUP);
  
  // Debug
  debugStart(io_USBSerialSpeed); // Start debugging
  // Blink while waiting for serial
  // Note: This while loop blocks everything else until a serial connection is found. If needed later, we can swap to FreeRTOS to handle this
  while(debugMode > 0 && !Serial) { 
    digitalWrite(LED_BUILTIN,1); delay(100);
    digitalWrite(LED_BUILTIN,0); delay(50);
  }

  // Load config data from NVS
  //todo

  // DPS310 setup (Barometric temp + pressure)
  debugMsg("\n\n\n[INIT]: Starting Logger\nInitializing sensors...");
  Wire.begin(p_SDA,p_SCL); // This is required because we can't use D5 (default) for I2C (hardware bug?)
  for (int i = 1; i <= 10; i++) { //Try up to 10 times to establish an I2C connection
    if (!dps.begin_I2C()) {
      debugMsg("  DPS310 - Failed I2C connection attempt ",1,0);
      debugMsg(i,1,1);
    } else { 
      dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES); // See DPS310 datasheet (PRS_CFG and TMP_CFG register information) for more info on sampling rates
      dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
      debugMsg("  DPS310 - Connected at ",1,0); debugMsg(Wire.getClock(),1,0); debugMsg("Hz");
      break;
    }
    if (i == 10) {
      debugMsg("  [CRITICAL]: Failed to initialize Sensor: DPS310");
    }
  }

  // ADXL377 setup (high g 3-axis accelerometer)
  //todo if needed
  
  
  debugMsg("[INIT]: Startup finished.\n");
}

// Loop -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {

  // Process Accelerometer Data
  if (millis() - io_accelSampleTimer >= io_accelSampleRate) { // If it's time to collect an accelerometer sample
    // Performance: the following calculations take approx 0.2ms to complete
    // unsigned long performanceTimer = micros();
    // Calculate data
    int x = analogRead(p_xAccel);
    int y = analogRead(p_yAccel);
    int z = analogRead(p_zAccel);

    // Store the data to sample arrays
    dat_xAccelSamples[io_accelCurrentSample] = x;
    dat_yAccelSamples[io_accelCurrentSample] = y;
    dat_zAccelSamples[io_accelCurrentSample] = z;

    // Increment the current sample number (reset to 0 if we've gone past the max sample array size)
    io_accelSampleTimer = millis(); // Reset the sample timer
    io_accelCurrentSample += 1;
    if (io_accelCurrentSample > (io_accelSamples - 1)) {
      io_accelCurrentSample = 0;
    }
    // performanceTimer = micros() - performanceTimer;
    // debugMsg("Accelerometer sample collected in (microsec): ",1,0); debugMsg(performanceTimer);

    // Calibrate the accelerometer if needed
    if (cal_accelCalMode) {
      if (!cal_accelCalStarted) { // If calibration mode was just started
        cal_accelCalStarted = 1; // Set started flag 
        cal_accelCalTimer = millis();
        // Clear +/- 1g calibration values
        cal_n1gXAccell = x; cal_n1gYAccell = y; cal_n1gZAccell = z;
        cal_p1gXAccell = x; cal_p1gYAccell = y; cal_p1gZAccell = z;
      }
      if (millis() - cal_accelCalTimer > cal_accelCalTimeout) { // Turn off calibration after 10 sec
        cal_accelCalMode = 0;
      }
      // If the current measured value is beyond one of the limits, save that as the new limit
      if (x < cal_n1gXAccell) cal_n1gXAccell = x;
      if (y < cal_n1gYAccell) cal_n1gYAccell = y;
      if (z < cal_n1gZAccell) cal_n1gZAccell = z;
      if (x > cal_p1gXAccell) cal_p1gXAccell = x;
      if (y > cal_p1gYAccell) cal_p1gYAccell = y;
      if (z > cal_p1gZAccell) cal_p1gZAccell = z;
      debugMsg("x,y,z min values: ",2,0); 
      debugMsg(cal_n1gXAccell,2,0); debugMsg(",",2,0); debugMsg(cal_n1gYAccell,2,0); debugMsg(",",2,0); debugMsg(cal_n1gZAccell,2,1);
      debugMsg("x,y,z max values: ",2,0); 
      debugMsg(cal_p1gXAccell,2,0); debugMsg(",",2,0); debugMsg(cal_p1gYAccell,2,0); debugMsg(",",2,0); debugMsg(cal_p1gZAccell,2,1); 
    }
    if (!cal_accelCalMode && cal_accelCalStarted) { // If calibration mode was just turned off
      cal_accelCalStarted = 0; // Clear the started flag
      // Calculate new coefficients and zero values
      cal_xAccelCoef = float(2) / (cal_p1gXAccell - cal_n1gXAccell); 
      cal_yAccelCoef = float(2) / (cal_p1gYAccell - cal_n1gYAccell); 
      cal_zAccelCoef = float(2) / (cal_p1gZAccell - cal_n1gZAccell); 
      cal_zeroXAccel = cal_p1gXAccell - ((cal_p1gXAccell - cal_n1gXAccell) / 2);
      cal_zeroYAccel = cal_p1gYAccell - ((cal_p1gYAccell - cal_n1gYAccell) / 2);
      cal_zeroZAccel = cal_p1gZAccell - ((cal_p1gZAccell - cal_n1gZAccell) / 2);
      //Todo: report calibration data to web interface w/ confirm option, if confirmed save to Preferences

      debugMsg("Final x,y,z min values: ",2,0); 
      debugMsg(cal_n1gXAccell,2,0); debugMsg(",",2,0); debugMsg(cal_n1gYAccell,2,0); debugMsg(",",2,0); debugMsg(cal_n1gZAccell,2,1);
      debugMsg("Final x,y,z max values: ",2,0); 
      debugMsg(cal_p1gXAccell,2,0); debugMsg(",",2,0); debugMsg(cal_p1gYAccell,2,0); debugMsg(",",2,0); debugMsg(cal_p1gZAccell,2,1); 
      debugMsg("x,y,z Coefficients: ",2,0); 
      debugMsg(cal_xAccelCoef,2,0); debugMsg(", ",2,0); debugMsg(cal_yAccelCoef,2,0); debugMsg(", ",2,0); debugMsg(cal_zAccelCoef,2,1); 
      debugMsg("x,y,z zero values: ",2,0); 
      debugMsg(cal_zeroXAccel,2,0); debugMsg(",",2,0); debugMsg(cal_zeroYAccel,2,0); debugMsg(",",2,0); debugMsg(cal_zeroZAccel,2,1); 
    }
  }
  
  // Process Altimeter Data
  sensors_event_t temp_event, pressure_event;
  if ((millis() - io_altSampleTimer >= io_altSampleRate) && (dps.temperatureAvailable() || dps.pressureAvailable())) { //If it's time to collect an altimeter sample and there's new altimeter data
    // Performance: the following calculations take approx 1.3ms to complete
    // unsigned long performanceTimer = micros();
    // Calculate data
    dps.getEvents(&temp_event, &pressure_event);
    dat_tempC = temp_event.temperature;
    dat_tempK = dat_tempC + 273.15;
    dat_tempF = dat_tempC * 1.8; dat_tempF += 32;
    dat_pressPa = pressure_event.pressure * 100;
    dat_altMBaro = pow((cal_pAtSea/dat_pressPa),cal_magicExp);
    dat_altMBaro -= 1; dat_altMBaro *= dat_tempK; dat_altMBaro /= cal_lapseRate;
    dat_altFtBaro = dat_altMBaro * 3.280839895;
    // Store the data to sample arrays
    dat_tempCSamples[io_altCurrentSample] = dat_tempC;
    dat_tempKSamples[io_altCurrentSample] = dat_tempK;
    dat_tempFSamples[io_altCurrentSample] = dat_tempF;
    dat_pressPaSamples[io_altCurrentSample] = dat_pressPa;
    dat_altMBaroSamples[io_altCurrentSample] = dat_altMBaro;
    dat_altFtBaroSamples[io_altCurrentSample] = dat_altFtBaro;
    // Increment the current sample number (reset to 0 if we've gone past the max sample array size)
    io_altSampleTimer = millis(); // Reset the sample timer
    io_altCurrentSample += 1;
    if (io_altCurrentSample > (io_altSamples - 1)) {
      io_altCurrentSample = 0;
    }
    // performanceTimer = micros() - performanceTimer;
    // debugMsg("Altimeter sample collected in (microsec): ",1,0); debugMsg(performanceTimer);
  }

  

  // Log fast-polled data
  if (millis() - io_logQuickTimer > io_logQuickTime && !cal_accelCalMode) { // If it's time to log data and we're not in calibration mode
    // Performance: the following logging routine takes approx TODOms to complete
    unsigned long performanceTimer = micros();
    io_logQuickTimer = millis(); // Reset the fast data logging timer

    //Average data in accelerometer arrays
    dat_xAccelRaw = 0; dat_yAccelRaw = 0; dat_zAccelRaw = 0; // Init all values to 0
    float x = 0, y = 0, z = 0;
    for (int i=0; i < io_accelSamples; i++) {
      dat_xAccelRaw += dat_xAccelSamples[i];
      dat_yAccelRaw += dat_yAccelSamples[i];
      dat_zAccelRaw += dat_zAccelSamples[i];
    }
    dat_xAccelRaw = dat_xAccelRaw / float(io_accelSamples); dat_yAccelRaw = dat_yAccelRaw / float(io_accelSamples); dat_zAccelRaw = dat_zAccelRaw / float(io_accelSamples);
    //Calculate g force and m/s^2 values
    // dat_xAccelG = (dat_xAccelRaw - cal_zeroXAccel) * cal_xAccelCoef;
    // dat_yAccelG = (dat_yAccelRaw - cal_zeroYAccel) * cal_yAccelCoef;
    // dat_zAccelG = (dat_zAccelRaw - cal_zeroZAccel) * cal_zAccelCoef;
    dat_xAccelG = mapf(dat_xAccelRaw, 0, 4095, float(-200), float(200));
    dat_yAccelG = mapf(dat_yAccelRaw, 0, 4095, float(-200), float(200));
    dat_zAccelG = mapf(dat_zAccelRaw, 0, 4095, float(-200), float(200));

    dat_xAccelMs2 = dat_xAccelG * 9.80665;
    dat_yAccelMs2 = dat_yAccelG * 9.80665;
    dat_zAccelMs2 = dat_zAccelG * 9.80665;

    // Average the data in the altimeter arrays
    dat_tempC = 0; dat_tempF = 0; dat_tempK = 0; dat_pressPa = 0; dat_altMBaro = 0; dat_altFtBaro = 0; // Init all values to 0
    for (int i=0; i < io_altSamples; i++) {
      dat_tempC += dat_tempCSamples[i]; 
      dat_tempF += dat_tempFSamples[i];
      dat_tempK += dat_tempKSamples[i];
      dat_pressPa += dat_pressPaSamples[i];
      dat_altMBaro += dat_altMBaroSamples[i];
      dat_altFtBaro += dat_altFtBaroSamples[i];
    }
    dat_tempC /= io_altSamples;
    dat_tempF /= io_altSamples;
    dat_tempK /= io_altSamples;
    dat_pressPa /= io_altSamples;
    dat_altMBaro /= io_altSamples;
    dat_altFtBaro /= io_altSamples;

    //Log to current log file in SD Card
      // TODO

    performanceTimer = micros() - performanceTimer;
    // debugMsg("Fast Log entry created in (microsec): ",1,0); debugMsg(performanceTimer);

    
    // Print to console
    // debugMsg("[DATA]: DPS310",2,1);
    debugMsg(">Temp(C): ",2,0); debugMsg(dat_tempC,2,1);
    debugMsg(">Temp(F): ",2,0); debugMsg(dat_tempF,2,1);
    debugMsg(">Temp(K): ",2,0); debugMsg(dat_tempK,2,1);
    debugMsg(">Pressure(Pa): ",2,0); debugMsg(dat_pressPa,2,1); 
    debugMsg(">Altitude(m): ",2,0); debugMsg(dat_altMBaro,2,1); 
    debugMsg(">Altitude(Ft): ",2,0); debugMsg(dat_altFtBaro,2,1); 
    // debugMsg("[DATA]: ADXL377",2,1);
    debugMsg(">X Accel (raw): ",2,0); debugMsg(dat_xAccelRaw,2,1);
    debugMsg(">X Accel (raw, single): ",2,0); debugMsg(dat_xAccelSamples[3],2,1);
    debugMsg(">X Accel (g): ",2,0); debugMsg(dat_xAccelG,2,1);
    debugMsg(">X Accel (m/s^2): ",2,0); debugMsg(dat_xAccelMs2,2,1);
    debugMsg(">Y Accel (raw): ",2,0); debugMsg(dat_yAccelRaw,2,1);
    debugMsg(">Y Accel (raw, single): ",2,0); debugMsg(dat_yAccelSamples[3],2,1);
    debugMsg(">Y Accel (g): ",2,0); debugMsg(dat_yAccelG,2,1);
    debugMsg(">Y Accel (m/s^2): ",2,0); debugMsg(dat_yAccelMs2,2,1);
    debugMsg(">Z Accel (raw): ",2,0); debugMsg(dat_zAccelRaw,2,1);
    debugMsg(">Z Accel (raw, single): ",2,0); debugMsg(dat_zAccelSamples[3],2,1);
    debugMsg(">Z Accel (g): ",2,0); debugMsg(dat_zAccelG,2,1);
    debugMsg(">Z Accel (m/s^2): ",2,0); debugMsg(dat_zAccelMs2,2,1);
  }


  // Log Slow-polled Data
  // if (millis() - io_logQuickTimer > io_logQuickTime) { //if it's time to log data
  //   // TODO: idk what we'd log here but it's an option if we want to log something at a lower rate
  // io_logQuickTimer = millis(); // Reset the slow data logging timer
  // }
    

  // Blink LED
  if ((millis() - io_StatLEDTimer) > 1000) {
    int switchTime = millis() - io_StatLEDTimer;
    io_StatLEDTimer = millis(); // Reset the state timer
    io_StatLEDState = !io_StatLEDState; // Toggle state
    digitalWrite(LED_BUILTIN,io_StatLEDState); // Write the state to the LED pin
    // debugMsg("[EVENT] Status LED ",2,0); 
    // if (io_StatLEDState) { 
    //   debugMsg("On (in ",2,0); 
    // } else {
    //  debugMsg("Off (in ",2,0);
    // } 
    // debugMsg(switchTime,2,0);
    // debugMsg("ms)",2,1);
    
  }
  
}