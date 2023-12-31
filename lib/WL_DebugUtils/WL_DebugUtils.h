#include <Arduino.h>
/*
  WL_DebugUtils.h
  Shorthand debug message functions to speed the debugging process.
  (Yes, it's poor form to have everything in the header file--that's fine because c++ doesn't care)

  The debugMode int must be defined prior to using any of these functions
    int debugMode = 1;

  debugMsg() accepts all types that Serial.print() does.

    
  TODO: It sure would be lovely if this project were running on c++17 or higher;
        then I could use <type_traits> with if constxpr (std::is_floating_point<Type>::value) to detect float and double types instead of manual type overloading 

  Written by Will's Lab, distributed freely without any license
*/
extern int debugMode; // You should define this as a number somewhere in your main code file!

/// @brief Start a serial connection (Serial.begin) if debugMode is >= 0. IMPORTANT: int debugMode must be declared globally prior to calling debugStart
/// @param baud Serial connection speed. Default: 9600
void debugStart(int baud = 9600) {
  if (debugMode > 0) {
    Serial.begin(baud);
  }
}

/// @brief Close the debug serial connection if debugMode is >=0
void debugStop() {
  if (debugMode > 0) {
    Serial.end();
  }
}

/// @brief Send a debug message to Serial depending on debugMode
/// @tparam Type 
/// @param txtIn variable or text to be sent (may be any type that's supported by Serial.print)
/// @param minLevel minimum debug level required for this message to be sent. Default: 1
/// @param ln if true, use Serial.println, else use Serial.print. Default: true
template <typename Type>
void debugMsg(Type txtIn, int minLevel = 1, bool ln = true) {
  if ((debugMode > 0) && (debugMode >= minLevel)) {
    if (ln) {
      Serial.println(txtIn);
    } else {
      Serial.print(txtIn);
    }
  }
}


// Special cases for float and double types:


/// @brief Send a debug message to Serial depending on debugMode
/// @tparam Type 
/// @param txtIn variable or text to be sent (may be any type that's supported by Serial.print)
/// @param minLevel minimum debug level required for this message to be sent. Default: 1
/// @param ln if true, use Serial.println, else use Serial.print. Default: true
/// @param decPrecision decimal precision for printing float / double types. Default: 8
void debugMsg(float txtIn, int minLevel = 1, bool ln = true, int decPrecision = 8) {
  if ((debugMode > 0) && (debugMode >= minLevel)) {
    if (ln) {
      Serial.println(txtIn, decPrecision);
    } else {
      Serial.print(txtIn, decPrecision);
    }
  }
}

/// @brief Send a debug message to Serial depending on debugMode
/// @tparam Type 
/// @param txtIn variable or text to be sent (may be any type that's supported by Serial.print)
/// @param minLevel minimum debug level required for this message to be sent. Default: 1
/// @param ln if true, use Serial.println, else use Serial.print. Default: true
/// @param decPrecision decimal precision for printing float / double types. Default: 8
void debugMsg(double txtIn, int minLevel = 1, bool ln = true, int decPrecision = 8) {
  if ((debugMode > 0) && (debugMode >= minLevel)) {
    if (ln) {
      Serial.println(txtIn, decPrecision);
    } else {
      Serial.print(txtIn, decPrecision);
    }
  }
}