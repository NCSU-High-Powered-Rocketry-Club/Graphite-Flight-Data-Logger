/* WebFuncs.cpp
    Functions for sending and processing web server data

    Web pages are stored as raw string literals in the web/webpage_name.h files for cleanliness. 
*/
#include <Arduino.h>
#include <WebServer.h>

void wi_NotFound(WebServer& server) {
    server.send(404, "text/plain", "Page / data not found");
    debugMsg("[EVENT]: Web Server sent 404 page");
}

void wi_sendStatus(WebServer& server) {
  // If the logger is armed, send the armed version status page. 
  // Having two different page versions is important because we don't want any scripts on the page requesting data from us if the logger is armed, because we need
  // as much RTOS headroom as possible to execute our launch detection logic reliably. Else we might mis the launch event by a few (or dozens of ) milliseconds
  debugMsg("[EVENT]: Client requested status page");
  unsigned long performanceTimer = millis();
  if (!flag_armed) { // Send non-armed status page
    File page = SPIFFS.open("/status.html", "r");
    if (!page) { 
      server.send(500, "text/plain", "File not found");
      debugMsg("[ERROR]: Couldn't open html file");
    } else {
      server.sendHeader("Cache-Control", "max-age=86400"); 
      server.streamFile(page, "text/html");
      page.close();
      performanceTimer = millis() - performanceTimer;
      debugMsg("[EVENT]: WebServer sent non-armed status page to client in ",1,0); debugMsg(performanceTimer,1,0); debugMsg("ms\n\n");
    }
  } else { // Send armed status page
    File page = SPIFFS.open("/statusArmed.html", "r");
    if (!page) {
      server.send(500, "text/plain", "File not found");
      debugMsg("[ERROR]: Couldn't open html file");
    } else {
      server.sendHeader("Cache-Control", "max-age=86400"); 
      server.streamFile(page, "text/html");
      page.close();
      performanceTimer = millis() - performanceTimer;
      debugMsg("[EVENT]: WebServer sent armed status page to client in ",1,0); debugMsg(performanceTimer,1,0); debugMsg("ms\n\n");
    }
    
  }
}

// For sending pages other than the status page
void wi_sendPage(WebServer& server, const char * fileName) { 
  if (!flag_armed) { // Only send the page if we aren't armed
    debugMsg("[EVENT]: Client requested a page: ",1,0); debugMsg(fileName);
    unsigned long performanceTimer = millis();
    File page = SPIFFS.open(fileName, "r");
    if (!page) {
      server.send(500, "text/plain", "File not found");
      debugMsg("[ERROR]: Couldn't open html file");
    } else {
      server.sendHeader("Cache-Control", "max-age=86400"); 
      server.streamFile(page, "text/html");
      page.close();
    }
    performanceTimer = millis() - performanceTimer;
    debugMsg("[EVENT]: WebServer sent",1,0); debugMsg(fileName,1,0); debugMsg(" to client in ",1,0); debugMsg(performanceTimer,1,0); debugMsg("ms\n\n");
  }
}




// Page interaction request functions


/// @brief Sync the internal RTC on the ESP32 with a time argument from the client (see data/status.html for corresponding js)
/// @param server WebServer object
void wi_syncTime(WebServer& server) {
  if (flag_armed) return; // Don't execute if we're armed for launch
  String clientTime = server.arg("plain");
  debugMsg("[EVENT]: Time sync sent from client:");
  debugMsg(clientTime);

  // Extract date and time from client arg, formatted as: "01/05/2024 22:00:38 GMT-0500 (Eastern Standard Time)" 
  // IMPORTANT: Month and day MUST have leading zeros (this is non-standard for js .toLocaleDateString()!
  time_month = clientTime.substring(0, 2).toInt(); 
  time_day = clientTime.substring(3, 5).toInt();   
  time_year = clientTime.substring(6, 10).toInt(); 
  time_hr = clientTime.substring(11, 13).toInt();  
  time_min = clientTime.substring(14, 16).toInt(); 
  time_sec = clientTime.substring(17, 19).toInt(); 
  time_zone = clientTime.substring(20, 28);
  // debugMsg(time_month,1,0); debugMsg(" | ",1,0); debugMsg(time_day,1,0); debugMsg(" | ",1,0); debugMsg(time_year,1,0); debugMsg(" | ",1,0);
  // debugMsg(time_hr,1,0); debugMsg(" | ",1,0); debugMsg(time_min,1,0); debugMsg(" | ",1,0); debugMsg(time_sec,1,0); debugMsg(" | ",1,0); debugMsg(time_zone);
  
  //TODO: Add some logic to verify if the time string we parsed above makes sense or if it's likely corrupt (rtc.setTime doesn't return anything or we could use that)
  //TODO: Store in RTC with ESP implementation instead of ESP32Time lib https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
  rtc.setTime(time_sec,time_min,time_hr,time_day,time_month,time_year,0);
  time_synced = 1;
  
  if (true) { // For now, we always set the RTC correctly!
    server.send(200, "text/plain", "success");
  } else {
    server.send(400, "text/plain", "failed");
  }
  
  if (debugMode < 1) return; // The rest of this is just debug stuff
  char clientTimeStr[200];
  sprintf(clientTimeStr, "(DD/MM/YYYY HH:MM:SS ZONE): %02d/%02d/%04d %02d:%02d:%02d %s", time_day,time_month,time_year,time_hr,time_min,time_sec,time_zone);

  debugMsg("  Translated to: ",1,0); debugMsg(clientTimeStr);
  debugMsg("  and internal RTC set to: ",1,0); debugMsg(rtc.getDateTime(),1,0); debugMsg(":",1,0); debugMsg(rtc.getMillis());
}

void wi_updateStatus(WebServer& server) {
  if (flag_armed) return; // Don't execute if we're armed
  debugMsg("[EVENT]: Client requested statusUpdate XML data");
  unsigned long performanceTimer = millis();

  String xml = "<data>";
  xml += "<time>" + rtc.getTime() + ":" + String(rtc.getMillis()) + "</time>";
  xml += "<date>" + rtc.getDate() + "</date>";
  xml += "<xAccel>" + String(dat_xAccelRaw) + "</xAccel>";
  xml += "<yAccel>" + String(dat_yAccelRaw) + "</yAccel>";
  xml += "<zAccel>" + String(dat_zAccelRaw) + "</zAccel>";
  xml += "<pressPa>" + String(dat_pressPa) + "</pressPa>";
  xml += "<tempC>" + String(dat_tempC) + "</tempC>";
  xml += "<tempF>" + String(dat_tempF) + "</tempF>";
  xml += "<altM>" + String(dat_altMBaro) + "</altM>";
  xml += "<altFt>" + String(dat_altFtBaro) + "</altFt>";
  xml += "<battV>" + String(dat_battV) + "</battV>";
  xml += "<launchDetectAltFt>" + String("dummy") + "</launchDetectAltFt>";
  xml += "<launchDetectXAccel>" + String("dummy") + "</launchDetectXAccel>";
  xml += "<launchDetectYAccel>" + String("dummy") + "</launchDetectYAccel>";
  xml += "<launchDetectZAccel>" + String("dummy") + "</launchDetectZAccel>";
  xml += "<flightLoggingTimeout>" + String("dummy") + "</flightLoggingTimeout>";
  xml += "<landedLoggingTimeout>" + String("dummy") + "</landedLoggingTimeout>";
  xml += "</data>";

  server.send(200, "text/xml", xml);

  performanceTimer = millis() - performanceTimer;
  debugMsg("[EVENT]: WebServer sent statusUpdate XML data to client in ",1,0); debugMsg(performanceTimer,1,0); debugMsg("ms\n\n");
}

void wi_armForLaunch(WebServer& server) {
  debugMsg("[EVENT]: Client sent arm command");
  if (flag_armed) return; // Don't execute if we're already armed
  if (!time_synced) { // Don't arm if the time hasn't been synced
    server.send(400, "text/plain", "Time not synced");
  } else {
    if (!true) {
      //TODO: add check to verify there's >n space on SD card to create a new logfile

      server.send(200, "text/plain", "SD Card Full");
    } else {
      //TODO: add other arming code here (switch to fast logging speed, enable launch detection logic)

      flag_armed = true; 
      server.send(200, "text/plain", "success");
      debugMsg("[EVENT]: Logger is armed for launch!");
    }
  }
}

void wi_disarm(WebServer& server) {
  debugMsg("[EVENT]: Client sent disarm command");
  if (!flag_armed) return; // Don't execute if we're not armed
  if (true) { // For now there's nothing that would stop us from disarming
    server.send(200, "text/plain", "success");
    flag_armed = false; 
    //TODO: Add function call to close current log file here
    debugMsg("[EVENT]: Logger has been disarmed by client");
  } else {
    server.send(400, "text/plain", "dummy error reason");
  }
}