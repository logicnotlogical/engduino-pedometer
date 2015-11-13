#include <EEPROM.h>
#include <Wire.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoButton.h>
#include <EngduinoLEDs.h>
#include <EngduinoSD.h>
#include <stdlib.h>

typedef enum {WAIT_FOR_BTN_PRESS, WAIT_FOR_TIME_PULSE, LOGGING, UPLOADING} dev_state;

// define some key codes
#define CR 13
#define LF 10

// constants
const int _I_CONN_TIMEOUT = 5000;
const float _F_ACCEL_THRESHOLD = 2.0;

// flashes the LEDs i_flashes times in clr_flashcolor colour.
void flashLEDs(colour clr_flashcolor, int i_flashes) {
  for (int i = 1; i < i_flashes; i++) {
    EngduinoLEDs.setAll(clr_flashcolor, 3);
    delay(200);
    EngduinoLEDs.setAll(0, 0, 0);
    delay(200);
  }
  EngduinoLEDs.setAll(clr_flashcolor, 3);
  delay(200);
  EngduinoLEDs.setAll(0, 0, 0);
}

// returns the magnitude of engduino's acceleration.
float get_accel_mag() {
  float f_accel[3];
  EngduinoAccelerometer.xyz(f_accel);
  float f_a = sqrt(f_accel[0] * f_accel[0] + \
              f_accel[1] * f_accel[1] + \
              f_accel[2] * f_accel[2]);
  if (f_a == NULL)
    return 0;
  else
    return f_a;
}

void setup() {
  EngduinoAccelerometer.begin();

  EngduinoLEDs.begin();
  EngduinoLEDs.setAll(6, 2, 0);

  EngduinoButton.begin();

  EngduinoSD.begin();

  Serial.begin(9600);
}

// persistent vars
dev_state curr_dev_state = WAIT_FOR_BTN_PRESS;

// io vars
unsigned long i_connection_start_time   = 0; // 0 is a flag to say that we are not connecting to anything.
char s_buffer[16]; //enough for the 10 digit epoch plus a little more.
byte i_buffer_length = 0;

// eeprom
int i_eeprom_ptr = 0; //points to first block of free space in EEPROM.

// logging vars
bool b_step_flag = false; // true when above threshold.
float f_accel_mag = 0.0; //magnitude of the accel forces.
bool b_step_up = false; //true only on odd activations (therefore step recorded only once)

void loop() {
  // We are waiting for the user to press the button to sync the Engduino's timer.
  if (curr_dev_state == WAIT_FOR_BTN_PRESS) {
    EngduinoLEDs.setAll(RED, 3);

    if (EngduinoButton.wasPressed()) {
      // Send init signal to computer. We shall wait _CONN_TIMEOUT ms for the connection.
      Serial.println("pedometer_get_time");
      EngduinoLEDs.setAll(6, 2, 0); // set LEDs to orange.
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
          //i_connection_start_time = atoi(s_buffer); // we're using the i_connection_time variable to save space.

          //EEPROM.put(i_eeprom_ptr,atol(s_buffer)); // put the start time into the eeprom.
          i_eeprom_ptr += sizeof(long);

          //log also the current offset from start of program in ms.
          //EEPROM.put(i_eeprom_ptr,millis());
          i_eeprom_ptr += sizeof(unsigned long);

          curr_dev_state = LOGGING;
          flashLEDs(GREEN, 2);
          Serial.write("pedometer_got_time\n");
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
    f_accel_mag = get_accel_mag();
    EngduinoLEDs.setLED(0, GREEN, 3);
    
    if (f_accel_mag != NULL && f_accel_mag > _F_ACCEL_THRESHOLD && !b_step_flag) {
      flashLEDs(BLUE, 1);
        Serial.print("Accel: ");
        Serial.println(f_accel_mag);
      b_step_up = !(b_step_up);
      
      // log the step here.
      if (b_step_up) {
        //log_step();
        b_step_flag = true;
      }
    }
    if (f_accel_mag < _F_ACCEL_THRESHOLD && b_step_flag) {
      b_step_flag = false;
    }
  }
}
