/// engduinoPedometer.pde
/// By Matthew Bell
/// Released under public domain
///
/// This simple sketch will read from the serial port.

import processing.serial.*;
import java.io.*;
import java.util.Scanner;

final int _CHAR_LF = 10;
final int _CHAR_CR = 13;

// runs a command, returns first line of output.
// Credit to http://stackoverflow.com/questions/792024/how-to-execute-system-commands-linux-bsd-using-java
String execCommand(String strCommand) {    	
	try {
		Process p = Runtime.getRuntime().exec(strCommand);//Windows command, use "ls -oa" for UNIX
		Scanner sc = new Scanner(p.getInputStream());    		
		if (sc.hasNext())
      return sc.nextLine();
    else
      return "";
	}
	catch (IOException e) {
		System.out.println(e.getMessage());
    return "";
	}
}

// constants
final int APP_MODE_TIMELIMIT = 5000;

// Custom structures.

// EnumAppMode defines the program's current state.
enum EnumAppMode {
  IDLE, SET_DEV_TIME, GET_DATA, PLOT_DATA
}

// Global objects.
PFont fontHelvetica;

Serial portArduino;
String strPortBuffer;
String strStatus = "Nothing to do.";
EnumAppMode appModeCurrent, appModeLast;
int intTimeSinceAppModeChange = 0;

void setup() {
  size(640, 480);
  fontHelvetica = createFont("HelveticaNeueLT-Light-48",48);
  textFont(fontHelvetica);

  // setup the port. later we will make this programmatic.
  String strPortName = Serial.list()[0];
  portArduino = new Serial(this, strPortName, 9600);
  portArduino.bufferUntil('\n');

  appModeCurrent = EnumAppMode.IDLE;
  background(255);
}

void serialEvent(Serial myPort) {
  appModeLast = appModeCurrent;
  background(255);

  if (intTimeSinceAppModeChange > APP_MODE_TIMELIMIT) {
    appModeCurrent = EnumAppMode.IDLE;
  }

  if (myPort.available() > 0) {
    strPortBuffer = myPort.readStringUntil('\n').trim();
    System.out.println("SERIAL GET: '"+strPortBuffer+"'");
    if (strPortBuffer.equals("pedometer_get_time")) {
      appModeCurrent = EnumAppMode.SET_DEV_TIME;
      // return the current time as unix format (seconds since epoch)
      System.out.println("getting system time...");
      myPort.write(execCommand("date +%s"));
      System.out.println("\""+execCommand("date +%s")+"\"");
    }
    else if (strPortBuffer.equals("pedometer_got_time")) {
      appModeCurrent = EnumAppMode.IDLE;
    }
    else if (strPortBuffer.equals("pedometer_send_data")) {
      myPort.write("pedometer_ok");
      appModeCurrent = EnumAppMode.GET_DATA;
    }
  }
  
  switch(appModeCurrent) {
    case IDLE: strStatus = "Nothing to do."; break;
    case SET_DEV_TIME: strStatus = "Synchronising Engduino..."; break;
    case GET_DATA: strStatus = "Receiving data from Engduino..."; break;
    case PLOT_DATA: strStatus = "Plotting data..."; break;
  }

  if (appModeCurrent == appModeLast) {
    intTimeSinceAppModeChange++;
  }
  else {
    intTimeSinceAppModeChange = 0;
  }
}

void draw() { 
  background(255);

  textSize(16);
  fill(0);
  text(strStatus,20,20);
}
