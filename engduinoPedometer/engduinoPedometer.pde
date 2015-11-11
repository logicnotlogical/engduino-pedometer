/// engduinoPedometer.pde
/// By Matthew Bell
/// Released under public domain
///
/// This simple sketch will read from the serial port.

import processing.serial.*;
import java.io.*;
import java.util.Scanner;

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
String strStatus = "";
EnumAppMode appModeCurrent, appModeLast;
int intTimeSinceAppModeChange = 0;

void setup() {
  size(640, 480);
  fontHelvetica = createFont("HelveticaNeueLT-Light-48",48);
  textFont(fontHelvetica);

  // setup the port. later we will make this programmatic.
  String strPortName = Serial.list()[0];
  portArduino = new Serial(this, strPortName, 9600);

  appModeCurrent = EnumAppMode.IDLE;
}

void draw() {
  appModeLast = appModeCurrent;
  background(255);

  if (intTimeSinceAppModeChange > APP_MODE_TIMELIMIT) {
    appModeCurrent = EnumAppMode.IDLE;
  }

  if (portArduino.available() > 0) {
    strPortBuffer = portArduino.readStringUntil('\n').replace("\n","");
    System.out.println("SERIAL GET: \""+strPortBuffer+"\"");
    if (strPortBuffer.equals("pedometer_get_time")) {
      appModeCurrent = EnumAppMode.SET_DEV_TIME;
      // return the current time as unix format (seconds since epoch)
      System.out.println("getting system time...");
      portArduino.write(execCommand("date +%s"));
    }
  }
  
  // draw stuff.
  textSize(16);

  fill(0);
  text(strStatus,20,20);
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
