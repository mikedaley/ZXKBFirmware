//  ZXKBFirmware v1.0
//  Copyright 2016 Mike Daley

#include <Keyboard.h>
#include "keys.h"

const int dataLines = 5;
const int addressLines = 8;
const int dataPin[dataLines] = {A0, A1, A2, A3, 15};          // The Pro-Micro does not have an A4 pin so using 15
const int address[addressLines] = {2, 3, 4, 5, 6, 7, 8, 9};
const int keyboardModeSpeakerPin = 10;                        // Speak will beep based on mode 1 = Spectrum, 2 = Fuse, 3 = PC
const int keyboardModeButtonPin = 14;                         

// Tracking when special keys that need us to send specific USB keyboard values have been pressed means that
// we can release them correctly without constantly sending Keyboard.release commands for those keys
bool symbolShiftPressed = false;
bool capsShiftPressed = false;

bool enterPressed = false;
bool deletePressed = false;
bool leftPressed = false;
bool rightPressed = false;
bool upPressed = false;
bool downPressed = false;

bool debug = true;
bool fullKeyboard = true;

bool keyboardModeButtonPressed = false;

enum {
  MODE_SPECTRUM = 1,
  MODE_FUSE = 2, 
  MODE_PC = 3
};
int keyboardMode;

// Spectrum 16k/14k keyboard matrix. This matrix layout matches the hardware layout
int spectrumKeyMap[addressLines][dataLines] = {
// 0---------------1-------------2------3------4  
  {KEY_1,          KEY_2,        KEY_3, KEY_4, KEY_5}, // 0
  {KEY_Q,          KEY_W,        KEY_E, KEY_R, KEY_T}, // 1
  {KEY_A,          KEY_S,        KEY_D, KEY_F, KEY_G}, // 2
  {KEY_0,          KEY_9,        KEY_8, KEY_7, KEY_6}, // 3
  {KEY_P,          KEY_O,        KEY_I, KEY_U, KEY_Y}, // 4
  {KEY_LEFT_SHIFT, KEY_Z,        KEY_X, KEY_C, KEY_V}, // 5
  {KEY_RETURN    , KEY_L,        KEY_K, KEY_J, KEY_H}, // 6
  {KEY_SPACE     , KEY_LEFT_ALT, KEY_M, KEY_N, KEY_B}, // 7
};

// Fuse keyboard matrix.  When running FUSE-SDL on the Pi, to get to options you need to 
// press F1 which is not on the Spectrum+ keyboard, so moving to Fuse keyboard mode (2xbeeps)
// means that press 1 through 9 provides F1 - F9, plus a couple of other keys such as enter
int fuseKeyMap[addressLines][dataLines] = {
// 0----------1-------2-------3-------4  
  {KEY_F1,    KEY_F2, KEY_F3, KEY_F4, KEY_F5},
  {KEY_ESC,   0,      0,      0,      0},
  {0,         0,      0,      0,      0},
  {KEY_F10,   KEY_F9, KEY_F8, KEY_F7, KEY_F6},
  {0,         0,      0,      0,      0},
  {0,         0,      0,      0,      0},
  {KEY_RETURN,        0,      0,      0, 0},
  {KEY_SPACE,         0,      0,      0, 0}  
};

// PC Normal keyboard matrix
int pcKeyMapNormal[addressLines][dataLines] = {
// 0---------------1-------------2------3------4  
  {KEY_1,          KEY_2,        KEY_3, KEY_4, KEY_5}, // 0
  {KEY_Q,          KEY_W,        KEY_E, KEY_R, KEY_T}, // 1
  {KEY_A,          KEY_S,        KEY_D, KEY_F, KEY_G}, // 2
  {KEY_0,          KEY_9,        KEY_8, KEY_7, KEY_6}, // 3
  {KEY_P,          KEY_O,        KEY_I, KEY_U, KEY_Y}, // 4
  {KEY_LEFT_SHIFT, KEY_Z,        KEY_X, KEY_C, KEY_V}, // 5
  {KEY_RETURN    , KEY_L,        KEY_K, KEY_J, KEY_H}, // 6
  {KEY_SPACE     , KEY_LEFT_ALT, KEY_M, KEY_N, KEY_B}, // 7
}; 

// PC Shifted keyboard matrix. 
int pcKeyMapShifted[addressLines][dataLines] = {
// 0---------------1-------------2----------------3-------------4  
  {KEY_1,          KEY_2,        KEY_3,           KEY_4,        KEY_LEFT_ARROW}, // 0
  {KEY_Q,          KEY_W,        KEY_E,           KEY_R,        KEY_T},          // 1
  {KEY_A,          KEY_S,        KEY_D,           KEY_F,        KEY_G},          // 2
  {KEY_0,          KEY_9,        KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW}, // 3
  {KEY_P,          KEY_O,        KEY_I,           KEY_U,        KEY_Y},          // 4
  {KEY_LEFT_SHIFT, KEY_Z,        KEY_X,           KEY_C,        KEY_V},          // 5
  {KEY_RETURN    , KEY_L,        KEY_K,           KEY_J,        KEY_H},          // 6
  {KEY_BACKSPACE , KEY_LEFT_ALT, KEY_M,           KEY_N,        KEY_B},          // 7
}; 

// PC Shifted keyboard matrix. 
int pcKeyMapSymbolShift[addressLines][dataLines] = {
// 0------------------1------------------2------------------3---------------4  
  {KEY_EXCLAM,        KEY_AT,            KEY_HASH,          KEY_DOLLAR,     KEY_PERCENT},    // 0
  {KEY_Q,             KEY_W,             KEY_E,             KEY_LESSTHAN,   KEY_GTRTHAN},    // 1
  {KEY_TILDA,         KEY_BAR,           KEY_BACKSLASH,     KEY_LEFT_BRACE, KEY_RIGHT_BRACE},// 2
  {KEY_UNDER,         KEY_RIGHT_BRACKET, KEY_LEFT_BRACKET,  KEY_SGL_QUOTE,  KEY_APERSAND},   // 3
  {KEY_DBL_QUOTE,     KEY_SEMICOLON,     KEY_I,             KEY_SQR_RIGHT,  KEY_SQR_LEFT},   // 4
  {KEY_LEFT_SHIFT,    KEY_COLON,         KEY_POUND,         KEY_QUESTION,   KEY_FRWDSLASH},  // 5
  {KEY_RETURN    ,    KEY_EQUALS,        KEY_PLUS,          KEY_MINUS,      KEY_H},          // 6
  {KEY_BACKSPACE ,    0,                 KEY_PERIOD,        KEY_COMMA,      KEY_ASTER},      // 7
}; 

// Array used to store the state of indiviaul keys that have been pressed
bool keyPressed[addressLines][dataLines] = {
  {false, false, false, false, false}, 
  {false, false, false, false, false}, 
  {false, false, false, false, false}, 
  {false, false, false, false, false}, 
  {false, false, false, false, false}, 
  {false, false, false, false, false}, 
  {false, false, false, false, false}, 
  {false, false, false, false, false}
};

// *** SETUP ***
void setup() {

  // Setup the keyboard default mode, LED and button pins
  pinMode(keyboardModeSpeakerPin, OUTPUT);
  digitalWrite(keyboardModeSpeakerPin, LOW);
  pinMode(keyboardModeButtonPin, INPUT);
  keyboardMode = MODE_SPECTRUM;

  // Set the address line pins to output and set HIGH
  for (int a = 0; a < addressLines; a++) {
    pinMode(address[a], OUTPUT);
    digitalWrite(address[a], HIGH);
  }

  // Set the data line pins to INPUT
  for (int d = 0; d < dataLines; d++) {
    pinMode(dataPin[d], INPUT);
  }

  // Start the keyboard
  Keyboard.begin();

  // Setup the serial port for debug info if debugging has been enabled
  if (debug) {
    Serial.begin(9600);
  }
  
}

// *** LOOP ***
void loop() {
  
  // Check of the keyboard mode button has been pressed, if so then register its press, otherwise 
  // check to see if it has registered a press and update the keyboard mode as necessary
  if (digitalRead(keyboardModeButtonPin) == LOW) {

    // If the button has not already be registered as pressed then register the press now
    if (!keyboardModeButtonPressed) {
      keyboardModeButtonPressed = true; 
    }
    
  } else if (keyboardModeButtonPressed) {
    
    // The keyboard mode button has been pressed so reset the pressed state and update the
    // keyboard mode and beep to signify which mode you are in.
    keyboardModeButtonPressed = false;

    // Cycle through the different keyboard modes
    keyboardMode = (keyboardMode + 1 > MODE_PC) ? MODE_SPECTRUM : keyboardMode + 1; 

    // Generate a beep for each keyboard mode so we know what mode has been selected
    for (int beeps = 0; beeps < keyboardMode; beeps++) {
      beep(keyboardModeSpeakerPin, 350, 100);
      delay(40);
    }
    
    if (debug) {
      Serial.print("Keyboardmode: ");
      Serial.println(keyboardMode);
    }
  }

  if (!fullKeyboard) return;
  
  // Loop through all the address lines
  for (int addrLine = 0; addrLine < addressLines; addrLine++) {

    // Set the current address line to LOW so that we can check for a LOW on the data line
    digitalWrite(address[addrLine], LOW);

    // Loop through all the data lines
    for (int dataLine = 0; dataLine < dataLines; dataLine++) {
      
      // Get the value of the current data line...
      int pressed = digitalRead(dataPin[dataLine]);

      // ...and if it's LOW then a key has been pressed
      if (pressed == LOW) {

        // Deal with special keys by setting flags so we know what has been pressed
        if (addrLine == 7 && dataLine == 1 && !symbolShiftPressed) { // Symbol Shift
          symbolShiftPressed = true;
        } else if (addrLine == 5 && dataLine == 0 && !capsShiftPressed) { // Caps Shift
          capsShiftPressed = true;
        }

        int outKey = 0;

        // Manage what is sent from the keyboard based on the keyboard mode
        switch (keyboardMode) {
          
          case MODE_SPECTRUM:
            outKey = spectrumKeyMap[addrLine][dataLine];            
            break;
            
          case MODE_FUSE:
            outKey = fuseKeyMap[addrLine][dataLine];            
            break;
            
          case MODE_PC:
            // In PC mode we use the shift key being pressed to read keys from a different matrix making it
            // easier to assign different keys to Spectrum keyboard keys e.g. BREAK = BACKSPACE
            if (capsShiftPressed) {
              // Pressing a dedicated arrow key on the Spectrum+ keyboard actually regiters a CAPS SHIFT and then the
              // appropriate number for the arrow e.g. 5, 6, 7, 8. In PC mode this CAPS SHIFT means than as the cursor
              // moves it highlights as well. To stop this happening we release the CAPS SHIFT before sending the arrow
              // key. You now can't highlight with arrow keys but correct movement seemed more important :O)
              if ((addrLine == 3 && dataLine == 2) || (addrLine == 3 and dataLine == 3) || 
                 (addrLine == 3 && dataLine == 4) || (addrLine == 0 && dataLine == 4)) {
                  Keyboard.release(KEY_LEFT_SHIFT);
                 }
              outKey = pcKeyMapShifted[addrLine][dataLine];            
            } else if (symbolShiftPressed) {
              Keyboard.release(KEY_LEFT_ALT);
              outKey = pcKeyMapSymbolShift[addrLine][dataLine];            
            } else {
              outKey = pcKeyMapNormal[addrLine][dataLine];            
            }
            break;
            
          default:
            if (debug) Serial.print("ERROR: Unknown keyboard mode");
        }

         // We know what has been pressed and taken the key stroke to be send from the appropriate
         // matrix array, so now send it and marked it as pressed in the keyPressed matrix array
        if (!keyPressed[addrLine][dataLine]) {
          if (debug) printDebug(addrLine, dataLine);
          Keyboard.press(outKey);
          keyPressed[addrLine][dataLine] = true;
        }

      // No key has been pressed so manage releasing any keys that were pressed
      } else {

        int outKey = 0;

        // Deal with special keys that may have been pressed previously set
        if (addrLine == 7 && dataLine == 1 && symbolShiftPressed) {      
          symbolShiftPressed = false;
        } else if (addrLine == 5 && dataLine == 0 && capsShiftPressed) {
          capsShiftPressed = false;
        } 
          
        // Based on the keyboard mode deal with releasing previously pressed keys
        switch (keyboardMode) {

          case MODE_SPECTRUM:
            outKey = spectrumKeyMap[addrLine][dataLine];            
            break;

          case MODE_FUSE:
            outKey = fuseKeyMap[addrLine][dataLine];            
            break;

          case MODE_PC:
            if (capsShiftPressed) { 
              outKey = pcKeyMapShifted[addrLine][dataLine];            
            } else if (symbolShiftPressed) {
              outKey = pcKeyMapSymbolShift[addrLine][dataLine];            
            } else {
              outKey = pcKeyMapNormal[addrLine][dataLine];            
            }
            break;

          default:
            if (debug) Serial.println("ERROR: Unknown keyboard mode");
        }

        // Check to see if the current matrix poition has a key press registered. If so then release
        // that key and mark it as released
        if (keyPressed[addrLine][dataLine]) {
          Keyboard.release(outKey);
          keyPressed[addrLine][dataLine] = false;
        } 
        
      }
    }

    // Set the address line back to HIGH
    digitalWrite(address[addrLine], HIGH);
  }

}

void beep(int pin, int duration, int frequency) {
  for (int x = 0; x < duration; x++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(frequency);
    digitalWrite(pin, LOW);
    delayMicroseconds(frequency);
  }
}

void printDebug(int addrLine, int dataLine) {
  int outKey = spectrumKeyMap[addrLine][dataLine];            
  Serial.print ("Addr Line: "); Serial.print(addrLine);
  Serial.print(" - ");
  Serial.print("Data Line: "); Serial.print(dataLine);
  Serial.print(" - ");
  Serial.print("Key: "); Serial.print(outKey);
  Serial.print(" - ");
  Serial.print((capsShiftPressed) ? "CAPS ON " : "CAPS OFF");
  Serial.print(" : ");
  Serial.println((symbolShiftPressed) ? "SYM ON" : "SYM OFF");  
}


