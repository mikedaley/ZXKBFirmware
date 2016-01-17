//  ZXKBFirmware v.05
//  Copyright 2016 Mike Daley

#include <Keyboard.h>

const int dataLines = 5;
const int addressLines = 8;
const int dataPin[dataLines] = {A0, A1, A2, A3, 15};          // The Pro-Micro does not have an A4 pin so using 15
const int address[addressLines] = {2, 3, 4, 5, 6, 7, 8, 9};
const int keyboardModeSpeakerPin = 10;                            // When LED is on keyboard is in special mode
const int keyboardModeButtonPin = 14;                         

// Tracking when special keys that need us to send specific USB keyboard values have been pressed means that
// we can release them correctly without constantly sending Keyboard.release commands for those keys
bool symbolShiftPressed = false;
bool capsShiftPressed = false;

// 
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

// Spectrum 16k/14k keyboard matrix
char spectrumKeyMap[addressLines][dataLines] = {
//  0   1   2   3   4  
  {'1','2','3','4','5'}, // 0
  {'q','w','e','r','t'}, // 1
  {'a','s','d','f','g'}, // 2
  {'0','9','8','7','6'}, // 3
  {'p','o','i','u','y'}, // 4
  {' ','z','x','c','v'}, // 5
  {' ','l','k','j','h'}, // 6
  {' ',' ','m','n','b'}, // 7
};

// Fuse keyboard matrix
int fuseKeyMap[addressLines][dataLines] = {
  {KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5},
  {KEY_ESC, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {KEY_F10, KEY_F9, KEY_F8, KEY_F7, KEY_F6},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {KEY_RETURN, 0, 0, 0, 0},
  {' ', 0, 0, 0, 0}  
};

// Fuse keyboard matrix
char pcKeyMap[addressLines][dataLines] = {
  {'1','2','3','4','5'},
  {'q','w','e','r','t'},
  {'a','s','d','f','g'},
  {'0','9','8','7','6'},
  {'p','o','i','u','y'},
  {' ','z','x','c','v'},
  {' ','l','k','j','h'},
  {' ',' ','m','n','b'},  
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

      // ...and if its LOW then a key has been pressed
      if (pressed == LOW) {

        // Deal with special keys
        if (addrLine == 7 && dataLine == 1) { // Symbol Shift
          if (symbolShiftPressed) continue;
          Keyboard.press(KEY_LEFT_ALT);
          symbolShiftPressed = true;
        } else if (addrLine == 5 && dataLine == 0) { // Caps Shift
          if (capsShiftPressed) continue;
          Keyboard.press(KEY_LEFT_SHIFT);
          capsShiftPressed = true;
        } else if (addrLine == 6 && dataLine == 0) {  // Enter
          if (enterPressed) continue;
           Keyboard.press(KEY_RETURN);
           enterPressed = true;
           if (debug) Serial.println("ENTER DOWN");
        } else {

          // Manage what is sent from the keyboard based on the keyboard mode
          switch (keyboardMode) {
            
            case MODE_SPECTRUM:
              if (!keyPressed[addrLine][dataLine]) {
                if (debug) printDebug(addrLine, dataLine);
                char outKey = spectrumKeyMap[addrLine][dataLine];            
                Keyboard.press(outKey);
                keyPressed[addrLine][dataLine] = true;
              }
              break;
              
            case MODE_FUSE:
              if (!keyPressed[addrLine][dataLine]) {
                if (debug) printDebug(addrLine, dataLine);
                int outKey = fuseKeyMap[addrLine][dataLine];            
                Keyboard.press(outKey);
                keyPressed[addrLine][dataLine] = true;
              }
              break;
              
            case MODE_PC:
              if (addrLine == 7 && dataLine == 0 && capsShiftPressed) {
                if (deletePressed) continue;
                Keyboard.press(KEY_BACKSPACE);
                deletePressed = true;
                if (debug) Serial.println("BACKSPACE DOWN");          
              } else if (addrLine == 3 && dataLine == 3 &  capsShiftPressed) {
                if (upPressed) continue;
                Keyboard.release(KEY_LEFT_SHIFT);
                Keyboard.press(KEY_UP_ARROW);
                upPressed = true;
                if (debug) Serial.println("UP DOWN");
              } else if (addrLine == 3 && dataLine == 4 &  capsShiftPressed) {
                if (downPressed) continue;
                Keyboard.release(KEY_LEFT_SHIFT);
                Keyboard.press(KEY_DOWN_ARROW);
                downPressed = true;
                if (debug) Serial.println("DOWN DOWN");
              } else if (addrLine == 0 && dataLine == 4 &  capsShiftPressed) {
                if (leftPressed) continue;
                Keyboard.release(KEY_LEFT_SHIFT);
                Keyboard.press(KEY_LEFT_ARROW);
                leftPressed = true;
                if (debug) Serial.println("LEFT DOWN");
              } else if (addrLine == 3 && dataLine == 2 &  capsShiftPressed) {
                if (rightPressed) continue;
                Keyboard.release(KEY_LEFT_SHIFT);
                Keyboard.press(KEY_RIGHT_ARROW);
                rightPressed = true;
                if (debug) Serial.println("RIGHT DOWN");
              } else {
                if (!keyPressed[addrLine][dataLine]) {
                  if (debug) printDebug(addrLine, dataLine);
                  int outKey = pcKeyMap[addrLine][dataLine];            
                  Keyboard.press(outKey);
                  keyPressed[addrLine][dataLine] = true;
                }
              }              
              break;
              
            default:
              if (debug) Serial.print("ERROR: Unknown keyboard mode");
          }
        }

      // No key has been pressed so manage releasing any keys that were pressed
      } else {

        // Deal with special keys that may have been pressed previously
        if (addrLine == 7 && dataLine == 1 && symbolShiftPressed) {      
          Keyboard.release(KEY_LEFT_ALT);
          symbolShiftPressed = false;
        } else if (addrLine == 5 && dataLine == 0 && capsShiftPressed) {
          Keyboard.release(KEY_LEFT_SHIFT);
          capsShiftPressed = false;
        } else if (addrLine == 6 && dataLine == 0 && enterPressed) {
          Keyboard.release(KEY_RETURN);
          enterPressed = false;
          if (debug) Serial.println("ENTER UP");
        } else {
          
          // Based on the keyboard mode deal with releasing previously pressed keys
          switch (keyboardMode) {

            case MODE_SPECTRUM:
              if (keyPressed[addrLine][dataLine]) {    
                char outKey = spectrumKeyMap[addrLine][dataLine];
                Keyboard.release(outKey);
                keyPressed[addrLine][dataLine] = false;
              }
              break;

            case MODE_FUSE:
              if (keyPressed[addrLine][dataLine]) {
                int outKey = fuseKeyMap[addrLine][dataLine];
                Keyboard.release(outKey);
                keyPressed[addrLine][dataLine] = false;
              }
              break;

            case MODE_PC:
              if (addrLine == 7 && dataLine == 0 && capsShiftPressed && deletePressed) {
                Keyboard.release(KEY_BACKSPACE);
                deletePressed = false;
                if (debug) Serial.println("BACKSPACE UP"); 
              } else if (addrLine == 3 && dataLine == 3 && capsShiftPressed && upPressed) {
                Keyboard.release(KEY_UP_ARROW);
                upPressed = false;
                if (debug) Serial.println("UP UP");
              } else if (addrLine == 3 && dataLine == 4 && capsShiftPressed && downPressed) {
                Keyboard.release(KEY_DOWN_ARROW);
                downPressed = false;
                if (debug) Serial.println("DOWN UP");
              } else if (addrLine == 0 && dataLine == 4 && capsShiftPressed && leftPressed) {
                Keyboard.release(KEY_LEFT_ARROW);
                leftPressed = false;
                if (debug) Serial.println("LEFT UP");
              } else if (addrLine == 3 && dataLine == 2 && capsShiftPressed && rightPressed) {
                Keyboard.release(KEY_RIGHT_ARROW);
                rightPressed = false;
                if (debug) Serial.println("RIGHT UP");
              } else if (keyPressed[addrLine][dataLine]) {
                int outKey = pcKeyMap[addrLine][dataLine];
                Keyboard.release(outKey);
                keyPressed[addrLine][dataLine] = false;
              } 
              break;

            default:
              if (debug) Serial.println("ERROR: Unknown keyboard mode");

          }
        }
      }
    }
 
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
  char outKey = spectrumKeyMap[addrLine][dataLine];            
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
