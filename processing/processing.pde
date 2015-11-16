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

//colors
final int _COLOR_BACKGROUND = 0xFFDEDEDE;
final int _COLOR_TEXT = 0xFF222222;
final int _COLOR_FOREGROUND = 0xFF008CBA;

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
		e.printStackTrace();
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

// these are used so that we have a range when it comes to
// plotting graphs. Luckily, for both axes, we know the lowest is 0.
int intDataCount = 0;
int intTimeMax = 0;
float floatAccelMax = 0.0f;

FileWriter fileDataFw;

/*
 * this function initialises the fileWriter for use later.
 */
void initialiseStreamReader() {
  try { 
      fileDataFw  = new FileWriter(sketchPath() + "/data.dat",false);
    }
    catch (IOException e) {
      e.printStackTrace();
    }
}


void setup() {
  size(640, 480);
  fontHelvetica = createFont("HelveticaNeueLT-Light-48",48);
  textFont(fontHelvetica);

  // setup the port. later we will make this programmatic.
  String strPortName = Serial.list()[0];
  portArduino = new Serial(this, strPortName, 9600);
  portArduino.bufferUntil('\n'); 
  
  appModeCurrent = EnumAppMode.IDLE;
  background(_COLOR_BACKGROUND);
}

void serialEvent(Serial myPort) {
  background(_COLOR_BACKGROUND);

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
      appModeCurrent = EnumAppMode.GET_DATA;
      initialiseStreamReader();
    }
    else if (strPortBuffer.equals("pedometer_end")) {
      try {
        fileDataFw.close();
      }
      catch (IOException e) {
        e.printStackTrace(); 
      }
      appModeCurrent = EnumAppMode.PLOT_DATA;
    }
    else {
      try {
        try {
          // setting ranges to make graphs better.
          String[] values = strPortBuffer.split(" ");
          if (values.length > 2) {
            throw new IOException();
          }
          float floatTemp = Float.parseFloat(values[1]);
           if (floatTemp > floatAccelMax) {
             floatAccelMax = floatTemp;
          }
          // data only recorded if NullExcpetion not thrown.
          fileDataFw.write(strPortBuffer + "\n");
          intDataCount++;
          intTimeMax = Integer.parseInt(values[0]);

         }
        catch (NullPointerException e) {
        }
      }
      catch (IOException e) {
        e.printStackTrace();
      }
    }
  }
 
 //what to print to status screen. 
  switch(appModeCurrent) {
    case IDLE: strStatus = "Nothing to do."; break;
    case SET_DEV_TIME: strStatus = "Synchronising Engduino..."; break;
    case GET_DATA: strStatus = "Receiving data from Engduino..."; break;
    case PLOT_DATA: strStatus = "Plotting data..."; break;
  }

}


void plotGraph() throws IOException {
  
  // open a bufferedreader to read the file.
  FileInputStream fstream = new FileInputStream(sketchPath() + "/data.dat");
  BufferedReader br = new BufferedReader(new InputStreamReader(fstream));
  
  String strLine;

  int intLastTime = 0;
  float floatLastAccel = 0.0f;

  int intSteps = 0;
  // on an accelerometer, someone taking a step will cross the threshold twice.
  boolean bStepActive = false;
  boolean bStepUp = false;
  float floatStepThreshold = 1.5f;
  
  // read each line from the bufferedReader.
  while ((strLine = br.readLine()) != null) {
    // no try/catch blocks here, we know the data is good!
    int intTime = Integer.parseInt(strLine.split(" ")[0]);
    float floatAccel = Float.parseFloat(strLine.split(" ")[1]); 
    
    //setup for first trapezium.
    if (intLastTime == 0) {
      floatLastAccel = floatAccel;
    }
    
    stroke(_COLOR_FOREGROUND);
    fill(_COLOR_FOREGROUND);
    System.out.println(Integer.toString(width));
    
    //       /|
    //      / |
    //     /  | 
    //    /   | b
    //    |   |
    //  a |   |
    //    |___|
    //    x1 h
    //
    //    Desciption of the trapezium we are drawing for each data point.


    int x1 = (intLastTime * width) / intTimeMax;
    int h =  ((intTime - intLastTime)*width) / intTimeMax;
    int a =  floor(floatLastAccel * (height / floatAccelMax));
    int b =  floor(floatAccel * (height / floatAccelMax));

    // draw the trapezia that define our graph.
    beginShape();
      vertex(x1, height);
      vertex(x1, height - a);
      vertex(x1 + h, height - b);
      vertex(x1 + h, height); 
    endShape();
    intLastTime = intTime;
    floatLastAccel = floatAccel;
    intDataIndex++;

    //step stuff.
    if (!(bStepActive) && (floatAccel > floatStepThreshold)) {
      bStepActive = true;
    }
    else if ((bStepActive) && (floatAccel < floatStepThreshold)) {
      bStepActive = false;
      if (!bStepUp) {
        bStepUp = true;
      }
      else {
        bStepUp = false;
        intSteps++;
      }
    }
  }

  br.close();
  
  //time labels
  fill(_COLOR_BACKGROUND);
  text("Min: 0", 10, height -20);

  text("Max: " + Integer.toString(intTimeMax),width-100, height -20);
  text("Time (s)", width - 60, height);

  text("Steps: " + Integer.toString(intSteps),width / 2 - 10, height - 20);

  fill(_COLOR_TEXT);
  text("Max: " + Float.toString(floatAccelMax), 10, 40);
  text("Acceleration (G)", 10, 60);
}


void draw() { 
  background(_COLOR_BACKGROUND);

  textSize(16);
  fill(_COLOR_TEXT);
  text(strStatus,20,20);

  if (appModeCurrent == EnumAppMode.PLOT_DATA) {
    try {
      plotGraph();
    }
    catch (IOException e) {
      e.printStackTrace();
    }
  }
}