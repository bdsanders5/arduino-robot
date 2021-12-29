
//define RC input connections (all attach interupt pins) -> mega attach interupt pins = 2, 3, 18, 19, 20, 21
int ch3Input = 21;
int ch4Input = 20;
int ch5Input = 19;
int ch6Input = 18;
int ch7Input = 2;
int ch8Input = 3;

//empty connections
int emptySwitchOutput1 = A4;
int emptySwitchOutput2 = A5;

//camera output connections
int cameraTXSwitchOutput = 45;
int camera1PowerOutput = A6;
int camera1SigalOutput = A9;
int camera2PowerOutput = A7;
int camera2SigalOutput = A10;
int camera3PowerOutput = A8;
int camera3SigalOutput = A11;

//trigger output connections
int pullTriggerOutput = 43;
//light switch output pin
int lightSwitchOutput = 44;
//solinoid output pin
int solenoidSwitchOutput = 39;
//Motor output Connections
int panUpOutput = 42;
int rotateRightOutput = 40;
int panDownOutput = 38;
int rotateLeftOutput = 41;


//initialize startup variables
int triggerSwitchState = 0;
int cameraTXSwitchState = 0;
int camera1SwitchState = 0;
int camera2SwitchState = 0;
int camera3SwitchState = 0;
int lightSwitchState = 0;
int solenoidSwitchState = 0;
int joyUpState = 0;
int joyRightState = 0;
int joyDownState = 0;
int joyLeftState = 0;
int driverMotorsStateX = 64;
int driverMotorsStateY = 192;

//initialize RC input variables & temp vars for calculations
volatile long ch7; 
volatile long count0; // temporary variable for ch7
volatile long ch8;
volatile long count1; // temporary variable for ch8
volatile long ch6;
volatile long count2; // temporary variable for ch6 
volatile long ch5; // servo value 
volatile long count3; // temporary variable for ch5
volatile long ch4; // servo value 
volatile long count4; // temporary variable for ch4
volatile long ch3; // servo value 
volatile long count5; // temporary variable for ch3

// Define controller motors Joystick Values - Start at 1500 (middle position)
int joyposVert = 1500;
int joyposHorz = 1500;

int mode = 0;
int noSignalLoopCounter = 0;

//set default camera to 2 - (front drive cam)
int cameraSelected = 2;



void setup() { 
  Serial.begin(9600);
  Serial2.begin(9600);
  //initialize drive motors off
  Serial2.write(0);
  initRCInputPins();
  initSwitchPins();
  initTiltPanPins();
} 

void loop() { 
  delay(100); 
  if ( ( (ch7 > 1250) && (ch7 < 1750) ) || (ch7 == 0) ) {
    noSignalLoopCounter++;
  } else {
    noSignalLoopCounter = 0;
  }
  if (noSignalLoopCounter > 5) {
    killRobot();
  }

  //set mode...
  setMode(); //0 = neutral, 1 = drive, 2 = armed / safety off...
  if (mode == 1) {
    controlDriverMotors();
  } else if (mode == 2) {
    controlTiltPan();
    controlSwitches();
    controlTrigger();
  } else {
    controlCameraSelector();
  }
   
}  



void initRCInputPins() {
  pinMode(ch3Input, INPUT);
  pinMode(ch4Input, INPUT);
  pinMode(ch5Input, INPUT);
  pinMode(ch6Input, INPUT);
  pinMode(ch7Input, INPUT);
  pinMode(ch8Input, INPUT);
  // Catch up and down 
  #define int0 (PINE & 0x10) // PIN 2  
  #define int1 (PINE & 0x20) // PIN 3
  #define int2 (PIND & 0b00001000) // PIN 18 
  #define int3 (PIND & 0b00000100) // PIN 19
  #define int4 (PIND & 0b00000010) //pin 20
  #define int5 (PIND & 0b00000001) //pin 21
  attachInterrupt(2,handleInterrupt_P21,CHANGE); // PIN 21
  attachInterrupt(3,handleInterrupt_P20,CHANGE); // PIN 20
  attachInterrupt(4,handleInterrupt_P19,CHANGE); // PIN 19
  attachInterrupt(5,handleInterrupt_P18,CHANGE); // PIN 18
  attachInterrupt(0,handleInterrupt_P2,CHANGE);  // PIN 2
  attachInterrupt(1,handleInterrupt_P3,CHANGE); // PIN 3
}


void initSwitchPins() {
  //initialize digital output pins to HIGH so we can sink any current supplied to them (they source current immediately when pinmode set)...
  digitalWrite(pullTriggerOutput, HIGH);
  digitalWrite(cameraTXSwitchOutput, HIGH);
  digitalWrite(camera1PowerOutput, HIGH);
  digitalWrite(camera1SigalOutput, HIGH);
  digitalWrite(camera2PowerOutput, HIGH);
  digitalWrite(camera2SigalOutput, HIGH);
  digitalWrite(camera3PowerOutput, HIGH);
  digitalWrite(camera3SigalOutput, HIGH);
  digitalWrite(lightSwitchOutput, HIGH);
  digitalWrite(solenoidSwitchOutput, HIGH);
  digitalWrite(emptySwitchOutput1, HIGH);
  digitalWrite(emptySwitchOutput2, HIGH);
  
  //set output pins...
  pinMode(pullTriggerOutput, OUTPUT);
  pinMode(cameraTXSwitchOutput, OUTPUT);
  pinMode(camera1PowerOutput, OUTPUT);
  pinMode(camera1SigalOutput, OUTPUT);
  pinMode(camera2PowerOutput, OUTPUT);
  pinMode(camera2SigalOutput, OUTPUT);
  pinMode(camera3PowerOutput, OUTPUT);
  pinMode(camera3SigalOutput, OUTPUT);
  pinMode(lightSwitchOutput, OUTPUT);
  pinMode(solenoidSwitchOutput, OUTPUT);
  pinMode(emptySwitchOutput1, OUTPUT);
  pinMode(emptySwitchOutput2, OUTPUT);

  //turn camera 2 on by default...
  digitalWrite(camera2PowerOutput, LOW);
  digitalWrite(camera2SigalOutput, LOW);

}


void initTiltPanPins() {
  //initialize digital output pins to HIGH so we can sink any current supplied to them (they source current immediately when pinmode set)...
  digitalWrite(panUpOutput, HIGH);
  digitalWrite(rotateRightOutput, HIGH);
  digitalWrite(panDownOutput, HIGH);
  digitalWrite(rotateLeftOutput, HIGH);
  //setup output pins...
  pinMode(panUpOutput, OUTPUT);
  pinMode(panDownOutput, OUTPUT);
  pinMode(rotateLeftOutput, OUTPUT);
  pinMode(rotateRightOutput, OUTPUT);
}

void setMode() {
  if (ch5 < 1200) { //top right toggle switch in up position...
    mode = 1; //drive
  } else if (ch5 > 1800) {
    mode = 2; //armed
  } else {
    mode = 0; //camera selection
  }

}

void controlCameraSelector() {
  //kill drive motors...
  Serial2.write(0);
  solenoidSwitchState = 0;
  digitalWrite(solenoidSwitchOutput, HIGH);
  //make sure camera TX is on...
  digitalWrite(cameraTXSwitchOutput, LOW);

  if (ch3 > 1600) {
    joyUpState = 1;
  } else {
    joyUpState = 0;
  }
  if ((ch3 < 1350) && (ch3 > 900)) {
    joyDownState = 1;
  } else {
    joyDownState = 0;
  }
  if (ch4 > 1600)  {
    joyRightState = 1;
  } else {
    joyRightState = 0;
  }
  if ( (ch4 < 1450) && (ch4 > 900) ) {
    joyLeftState = 1;
  } else {
    joyLeftState = 0;
  }
  if (joyUpState == 1) {
    if (cameraSelected != 1) {
      killAllCameras();
      cameraSelected = 1;
    }
    digitalWrite(camera1PowerOutput, LOW);
    digitalWrite(camera1SigalOutput, LOW);
  }
  if (joyLeftState == 1) {
    if (cameraSelected != 2) {
      killAllCameras();
      cameraSelected = 2;
    }
    digitalWrite(camera2PowerOutput, LOW);
    digitalWrite(camera2SigalOutput, LOW);
  }
  if (joyDownState == 1) {
    if (cameraSelected != 3) {
      killAllCameras();
      cameraSelected = 3;
    }
    digitalWrite(camera3PowerOutput, LOW);
    digitalWrite(camera3SigalOutput, LOW);
  }
}


void controlTiltPan() {
  //kill drive motors...
  Serial2.write(0);
  solenoidSwitchState = 0;
  digitalWrite(solenoidSwitchOutput, HIGH);

  if (ch3 > 1600) {
    joyUpState = 1;
  } else {
    joyUpState = 0;
  }
  if ((ch3 < 1350) && (ch3 > 900)) {
    joyDownState = 1;
  } else {
    joyDownState = 0;
  }
  if (ch4 > 1600)  {
    joyRightState = 1;
  } else {
    joyRightState = 0;
  }
  if ( (ch4 < 1450) && (ch4 > 900) ) {
    joyLeftState = 1;
  } else {
    joyLeftState = 0;
  }
  
  if (joyUpState == 1) {
    digitalWrite(panUpOutput, LOW);
  } else {
    digitalWrite(panUpOutput, HIGH);
  }
  if (joyRightState == 1) {
    digitalWrite(rotateRightOutput, LOW);
  } else {
    digitalWrite(rotateRightOutput, HIGH);
  }
  if (joyDownState == 1) {
    digitalWrite(panDownOutput, LOW);
  } else {
    digitalWrite(panDownOutput, HIGH);
  }
  if (joyLeftState == 1) {
    digitalWrite(rotateLeftOutput, LOW);
  } else {
    digitalWrite(rotateLeftOutput, HIGH);
  }

}



void controlSwitches() {
  //ch6 = top right rotation thing // clockwise = 1983 //counter clockwise = 990
  if (ch6 > 1500) {
    cameraTXSwitchState = 0;
  } else {
    cameraTXSwitchState = 1;
  }
  //ch8 = top left rotation thing // clockwise = 1983 //counter clockwise = 990
  if (ch8 > 1500) {
    lightSwitchState = 1;
  } else {
    lightSwitchState = 0;
  }

  if (cameraTXSwitchState == 1) {
    digitalWrite(cameraTXSwitchOutput, LOW);
    if (cameraSelected == 2) {
      digitalWrite(camera2PowerOutput, LOW);
      digitalWrite(camera2SigalOutput, LOW);
    } else if(cameraSelected == 1) {
      digitalWrite(camera1PowerOutput, LOW);
      digitalWrite(camera1SigalOutput, LOW);
    } else {
      digitalWrite(camera3PowerOutput, LOW);
      digitalWrite(camera3SigalOutput, LOW);
    }
  } else {
    digitalWrite(cameraTXSwitchOutput, HIGH);
    killAllCameras();
  }
  if (lightSwitchState == 1) {
    digitalWrite(lightSwitchOutput, LOW);
  } else {
    digitalWrite(lightSwitchOutput, HIGH);
  }
  
  if (solenoidSwitchState == 1) {
    digitalWrite(solenoidSwitchOutput, LOW);
  } else {
    digitalWrite(solenoidSwitchOutput, HIGH);
  }
  
}



void controlTrigger() {
  if (ch5 > 1200) { //top right toggle switch in down position...
    //ch7 = upper left toggle // up 990 , down 1982
    if (ch7 > 1500) {
      triggerSwitchState = 1;
    } else {
      triggerSwitchState = 0;
    }
    if (triggerSwitchState == 1) {
      digitalWrite(pullTriggerOutput, LOW);
      delay(50);
    } else {
      digitalWrite(pullTriggerOutput, HIGH);
    }
  } else {
      digitalWrite(pullTriggerOutput, HIGH);
  }
}






void controlDriverMotors() {
  
  //disable trigger...
  triggerSwitchState = 0;
  digitalWrite(pullTriggerOutput, HIGH);
  //disable tilt/pan...
  joyUpState = 0;
  joyDownState = 0;
  joyRightState = 0;
  joyLeftState = 0;
  digitalWrite(panUpOutput, HIGH);
  digitalWrite(rotateRightOutput, HIGH);
  digitalWrite(panDownOutput, HIGH);
  digitalWrite(rotateLeftOutput, HIGH);

  // drivetrain pin on...
  digitalWrite(solenoidSwitchOutput, LOW);

  joyposVert = ch3;
  joyposHorz = ch4;
  int centerPositionVert = 1500;
  int centerPositionHorz = 1492;
  int signalBuffer = 25;

  if ((joyposVert > 750) && (joyposHorz > 750)) {
    if (joyposVert < (centerPositionVert - signalBuffer)) {
      // This is Backward
      //Determine Motor Speeds
      // As we are going backwards we need to reverse readings
      driverMotorsStateX = map(joyposVert, 1500, 1000, 64, 1); // motor 1... 64 is stopped, 1 is full reverse...
      driverMotorsStateY = map(joyposVert, 1500, 1000, 192, 128); // motor 2... 192 is stopped, 128 is full reverse
    } else if (joyposVert > (centerPositionVert + signalBuffer)) {
      // This is Forward
      //Determine Motor Speeds
      driverMotorsStateX = map(joyposVert, 1500, 2000, 64, 127);// motor 1... 64 is stopped, 127 full forward...
      driverMotorsStateY = map(joyposVert, 1500, 2000, 192, 255); // motor 2... 192 is stopped, 255 full forward.
    } else {
      // This is Stopped
      driverMotorsStateX = 64;
      driverMotorsStateY = 192;
    }


  // Now do the steering
  // The Horizontal position will "weigh" the motor speed
  // Values for each motor
    if (joyposHorz < (centerPositionHorz - signalBuffer)) {
      // Move Left
      // As we are going left we need to reverse readings
      // Map the number to a value of 64 maximum
      joyposHorz = map(joyposHorz, 1492, 992, 0, 64);
      driverMotorsStateX = driverMotorsStateX - joyposHorz;
      driverMotorsStateY = driverMotorsStateY + joyposHorz;
      // Don't exceed max range for motor speeds
      if (driverMotorsStateX < 0)driverMotorsStateX = 0;
      if (driverMotorsStateY > 255)driverMotorsStateY = 255;
    } else if (joyposHorz > (centerPositionHorz + signalBuffer)) {
      // Move Right
      // Map the number to a value of 64 maximum
      joyposHorz = map(joyposHorz, 1492, 1984, 0, 64);
      driverMotorsStateX = driverMotorsStateX + joyposHorz;
      driverMotorsStateY = driverMotorsStateY - joyposHorz;
      // Don't exceed max range for motor speeds
      if (driverMotorsStateX > 127)driverMotorsStateX = 127;
      if (driverMotorsStateY < 0)driverMotorsStateY = 0;      
    }

    if (driverMotorsStateX == 0) {
      driverMotorsStateX = 64;
    }
    if (driverMotorsStateY == 0) {
      driverMotorsStateY = 192;
    }

    // Set Motor Direction
    Serial2.write(driverMotorsStateX);
    Serial2.write(driverMotorsStateY);
    
  }
  
}










//helper functions...

void killAllCameras() {
  digitalWrite(camera1PowerOutput, HIGH);
  digitalWrite(camera1SigalOutput, HIGH);
  digitalWrite(camera2PowerOutput, HIGH);
  digitalWrite(camera2SigalOutput, HIGH);
  digitalWrite(camera3PowerOutput, HIGH);
  digitalWrite(camera3SigalOutput, HIGH);
}

void killRobot() {
  Serial2.write(0);
  digitalWrite(solenoidSwitchOutput, HIGH);
  digitalWrite(pullTriggerOutput, HIGH);
  //digitalWrite(cameraTXSwitchOutput, HIGH); leave a camera on
  digitalWrite(lightSwitchOutput, HIGH);
  digitalWrite(solenoidSwitchState, HIGH);
  digitalWrite(panUpOutput, HIGH);
  digitalWrite(rotateRightOutput, HIGH);
  digitalWrite(panDownOutput, HIGH);
  digitalWrite(rotateLeftOutput, HIGH);

}







void handleInterrupt_P2() { 
  if(int0) 
    count0=micros(); // we got a positive edge 
  else 
   ch7=micros()-count0; // Negative edge: get pulsewidth 
}

void handleInterrupt_P3() { 
  if(int1) 
    count1=micros(); // we got a positive edge 
  else 
   ch8=micros()-count1; // Negative edge: get pulsewidth 
} 

void handleInterrupt_P18() { 
  if(int2) 
    count2=micros(); // we got a positive edge 
  else 
   ch6=micros()-count2; // Negative edge: get pulsewidth 
}

void handleInterrupt_P19() { 
  if(int3) 
    count3=micros(); // we got a positive edge 
  else 
   ch5=micros()-count3; // Negative edge: get pulsewidth 
}


void handleInterrupt_P20() { 
  if(int4) 
    count4=micros(); // we got a positive edge 
  else 
   ch4=micros()-count4; // Negative edge: get pulsewidth 
}

void handleInterrupt_P21() { 
  if(int5) 
    count5=micros(); // we got a positive edge 
  else 
   ch3=micros()-count5; // Negative edge: get pulsewidth 
}
