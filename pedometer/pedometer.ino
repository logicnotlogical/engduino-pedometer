#include <Wire.h>
#include <EngduinoLEDs.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoButton.h>
#include <stdlib.h>

typedef enum {WAIT_FOR_BTN_PRESS, WAIT_FOR_TIME_PULSE, LOGGING, UPLOADING} dev_state;

// define this
#define CR 13
#define LF 10

// constants
const int _I_CONN_TIMEOUT = 5000;
const float _F_THRESHOLD = 1.7;

void flashLEDs(colour clr_flashcolor, int i_flashes) {
  for (int i = 0; i < i_flashes; i++) {
    EngduinoLEDs.setAll(clr_flashcolor,3);
    delay(200);
    EngduinoLEDs.setAll(0,0,0);
    delay(200);
  }
}

void setup() {
  EngduinoAccelerometer.begin();
  
  EngduinoLEDs.begin();
  EngduinoLEDs.setAll(6,2,0);
  
  EngduinoButton.begin();

  Serial.begin(9600);
}

// persistent vars
dev_state curr_dev_state  = WAIT_FOR_BTN_PRESS;
long unsigned int i_connection_start_time   = 0; // 0 is a flag to say that we are not connecting to anything.
char s_buffer[16]; //enough for the 10 digit epoch plus a little more.
byte i_buffer_length = 0;

void loop() {
  // We are waiting for the user to press the button to sync the Engduino's timer.
  if (curr_dev_state == WAIT_FOR_BTN_PRESS) {
    EngduinoLEDs.setAll(RED,3);
    
    if (EngduinoButton.wasPressed()) {
      // Send init signal to computer. We shall wait _CONN_TIMEOUT ms for the connection.
      Serial.write("pedometer_get_time\n");
      EngduinoLEDs.setAll(6,2,0); // set LEDs to orange.
      curr_dev_state = WAIT_FOR_TIME_PULSE;
      i_connection_start_time = millis();
    }
  }

  // Waiting on the time pulse from the system, epoch format.
  else if (curr_dev_state == WAIT_FOR_TIME_PULSE) {
    if ((millis() - i_connection_start_time) < _I_CONN_TIMEOUT) { // if it hasn't yet been more than _CONN_TIMEOUT ms since starting the connection
      if (Serial.available()) {
        //Grab data to buffer
        i_buffer_length = Serial.readBytesUntil(CR, s_buffer, 15);
        s_buffer[i_buffer_length] = '\0'; // null termination.
        if (isDigit(s_buffer[0])) {
          i_connection_start_time = atoi(s_buffer); // we're using the i_connection_time variable to save space.
          curr_dev_state = LOGGING;
          flashLEDs(GREEN,2);          
        }
      }
    }

    else { //out of time.
      curr_dev_state = WAIT_FOR_BTN_PRESS;
      // flush the input buffer.
      s_buffer; i_buffer_length = 0;
    }
  }

  // Logging
  if (curr_dev_state == LOGGING) {
    EngduinoLEDs.setLED(0,GREEN,3);
    
  }
}
