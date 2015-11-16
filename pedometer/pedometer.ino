#include <EEPROM.h>
#include <Wire.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoButton.h>
#include <EngduinoLEDs.h>
#include <EngduinoSD.h>
#include <stdlib.h>

typedef enum {WAIT_FOR_BTN_INIT, WAIT_FOR_TIME_PULSE, LOGGING, WAIT_FOR_BTN_SEND, WAIT_FOR_UPLOAD, UPLOADING, WAIT_FOR_CONFIRM} dev_state;

// define some key codes
#define CR 13
#define LF 10

// CONSTANTS
const int _I_CONN_TIMEOUT = 5000;
const float _F_ACCEL_THRESHOLD = 2.2;


// GLOBAL VARIABLES
dev_state curr_dev_state = WAIT_FOR_BTN_INIT;

// io vars
unsigned int i_connection_start_time   = 0; // 0 is a flag to say that we are not connecting to anything.
unsigned long i_dev_start_time = 0; //start time of logging according to millis()
unsigned long i_real_start_time = 0; //start time of logging sequenc according to a real clock.
char s_buffer[16]; //enough for the 10 digit epoch plus a little more.
byte i_buffer_length = 0;

// Data
volatile unsigned char regTmp; //temp register value storage.

// logging vars
bool b_step_flag = false; // true when above threshold.
float f_accel_mag = 0.0; //ma1gnitude of the accel forces.
float f_last_accel_mag = 0.0;
bool b_step_up = false; //true only on odd activations (therefore step recorded only once)



// FUNCTIONS

// removes the given file.
boolean sd_remove_file(char *filepath)
{
  regTmp = TIMSK4;
  TIMSK4  = 0x00; // Disable TMR4 interrupts (Used by LEDs).
  if (SD.remove(filepath)) {
    TIMSK4 = regTmp;
    return true;
  }
  else {
    TIMSK4 = regTmp;
    return false;
  }
}

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

void log_data(float f_accel, unsigned long i_time) {
  if (f_accel != NULL && f_accel > 0) {
    String s_buffer;
    i_time -= i_dev_start_time;
    s_buffer += i_time;
    s_buffer += " ";
    s_buffer += (floor(f_accel * 100) / 100);
    EngduinoSD.writeln(s_buffer);
  }
}


bool upload_connect() {
  Serial.println("pedometer_send_data");
  i_connection_start_time = millis();
  while (!Serial.available() && ((millis() - i_connection_start_time) < _I_CONN_TIMEOUT)) {
    delay(10);
  }
  if (Serial.available()) {
    i_buffer_length = Serial.readBytesUntil(CR, s_buffer, 15);
    
    if (strcmp(s_buffer, "pedometer_ok") == 0)
      return true;
    else {
      flashLEDs(RED, 2);
      return false;
    }
  }
  else
    return false;
}

void led_upload_pattern(int i) {
  // ARRAY LITERALS
  colour arr_led_0[16] = {OFF,OFF,OFF,OFF,OFF,OFF,CYAN,CYAN,CYAN,CYAN,OFF,OFF,OFF,OFF,OFF,OFF};
  colour arr_led_1[16] = {OFF,OFF,OFF,OFF,CYAN,CYAN,OFF,OFF,OFF,OFF,CYAN,OFF,OFF,OFF,OFF,OFF};
  colour arr_led_2[16] = {OFF,OFF,OFF,CYAN,OFF,OFF,OFF,OFF,OFF,OFF,OFF,CYAN,OFF,OFF,OFF,OFF};
  colour arr_led_3[16] = {OFF,CYAN,CYAN,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,CYAN,OFF,OFF,OFF};
  colour arr_led_4[16] = {CYAN,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,CYAN,CYAN,CYAN};
  
  switch (i) {
    case 0: EngduinoLEDs.setLEDs(arr_led_0);
        break;
    case 1: EngduinoLEDs.setLEDs(arr_led_1);
        break;
    case 2: EngduinoLEDs.setLEDs(arr_led_2);
        break;
    case 3: EngduinoLEDs.setLEDs(arr_led_3);
        break;
    case 4: EngduinoLEDs.setLEDs(arr_led_4);
        break;
  }
}

void upload_data() {
  unsigned int i_line_count = 0;
  char c_read_buffer;
  EngduinoSD.open("data.dat", FILE_READ);
  delay(800);
  while (EngduinoSD.available()) {    
    c_read_buffer = EngduinoSD.read();
      //led_upload_pattern(i_line_count % 5);
      //if (c_read_buffer == LF)
        //i_line_count++;
      Serial.print(c_read_buffer);
      delay(10);
  }
  Serial.println("pedometer_data_eof");
  EngduinoSD.close();
}


void setup() {
  EngduinoAccelerometer.begin();

  EngduinoLEDs.begin();
  //EngduinoLEDs.setAll(6, 2, 0);

  EngduinoButton.begin();

  // This opens the data file with the following params:
  // O_WRONLY = write mode
  // O_CREAT = create the file if it does not exist
  // O_TRUNC = overwrite any existing file.
  
  // Setup SD  
  EngduinoSD.begin();
  
 

  Serial.begin(9600);
}




void loop() {

  // IF CLAUSES IN REVERSE CHRONILOGICAL ORDER


  if (curr_dev_state == UPLOADING) {
    EngduinoLEDs.setAll(CYAN);
    upload_data();
    curr_dev_state = WAIT_FOR_BTN_INIT;
  }


  // WAITING FOR COMPUTER TO REPLY
  if(curr_dev_state == WAIT_FOR_UPLOAD) {
    EngduinoLEDs.setAll(4,0,4);
    if (upload_connect())
      curr_dev_state = UPLOADING;
    else
      curr_dev_state = WAIT_FOR_BTN_SEND;
  }


  // WAITING FOR USER TO INSTANTIATE UPLOAD
  if (curr_dev_state == WAIT_FOR_BTN_SEND) {
    EngduinoLEDs.setAll(BLUE,3);
    if (EngduinoButton.wasPressed()) {
      curr_dev_state = WAIT_FOR_UPLOAD;
      EngduinoLEDs.setAll(4,0,4);
    }
  }


  // LOGGING DATA
  if (curr_dev_state == LOGGING) {
    f_last_accel_mag = f_accel_mag;
    f_accel_mag = get_accel_mag();
    EngduinoLEDs.setLED(0, GREEN, 3); 
    log_data(f_accel_mag,millis());
    
//    if (f_accel_mag != NULL && f_accel_mag > _F_ACCEL_THRESHOLD && !b_step_flag) {      
////        Serial.print("Accel: ");
////        Serial.println(f_accel_mag);  
//      
//      // log the step here.
//      if (f_accel_mag > f_last_accel_mag && b_step_up) {
//        //log_step();
//        //Serial.println("step");
//        b_step_flag = true;
//      }
//      b_step_up = !(b_step_up);
//      //Serial.println(b_step_up);
//    }
//    if (f_accel_mag < _F_ACCEL_THRESHOLD && b_step_flag) {
//      b_step_flag = false;
//    }

      if (EngduinoButton.wasPressed()) {
        // close the file and reopen for reading.
        EngduinoSD.close();
        
        EngduinoLEDs.setAll(BLUE,3);
        curr_dev_state = WAIT_FOR_BTN_SEND;        
      }
    delay(20);
  }


  // Waiting on the time pulse from the system, epoch format.
  else if (curr_dev_state == WAIT_FOR_TIME_PULSE) {
    if ((millis() - i_connection_start_time) < _I_CONN_TIMEOUT) { // if it hasn't yet been more than _CONN_TIMEOUT ms since starting the connection
      if (Serial.available()) {
        //Grab data to buffer
        i_buffer_length = Serial.readBytesUntil(CR, s_buffer, 15);
        s_buffer[i_buffer_length] = '\0'; // null termination.

        if (isDigit(s_buffer[0])) {
          i_dev_start_time = millis();
          sd_remove_file("data.dat");
          delay(50);
          EngduinoSD.open("data.dat", FILE_WRITE);
          EngduinoSD.write("real_start_time: ");
          EngduinoSD.writeln(s_buffer);
//          EngduinoSD.write("real_start_time: ");
//          EngduinoSD.writeln(s_buffer);

          curr_dev_state = LOGGING;
          flashLEDs(GREEN, 2);
          Serial.println("pedometer_got_time");
        }
      }
    }

    else { //out of time.
      curr_dev_state = WAIT_FOR_BTN_INIT;
      // flush the input buffer.
      s_buffer; i_buffer_length = 0;
    }
  }

    
  // We are waiting for the user to press the button to sync the Engduino's timer.
  if (curr_dev_state == WAIT_FOR_BTN_INIT) {
    EngduinoLEDs.setAll(RED, 3);

    if (EngduinoButton.wasPressed()) {
      // Send init signal to computer. We shall wait _CONN_TIMEOUT ms for the connection.
      Serial.println("pedometer_get_time");
      
      EngduinoLEDs.setAll(6, 2, 0); // set LEDs to orange.
      curr_dev_state = WAIT_FOR_TIME_PULSE;
      i_connection_start_time = millis();
    }
  }
}
