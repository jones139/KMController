
#include "AccelStepper.h"

#define LS1_PIN 3
#define LS2_PIN 2
#define STEP_PIN 4
#define DIRECTION_PIN 5

#define DT 0.1   // Interface polling period (Sec)

#define SPEED_MIN 31    // Limit of arduino tone library
#define SPEED_MAX 10000  // Motor doesn't like being stepped any faster.
#define SPEED_INIT 500
#define HOME_SPEED 500
#define HOME_ACC 1000
#define ACC_MIN 1
#define ACC_MAX 30000
#define ACC_INIT 1000

#define MODE_STOP 0
#define MODE_MOVETO 1
#define MODE_CYCLE 2

#define DIR_REV 0
#define DIR_FWD 1

#define CYCLE_MIN 0
#define CYCLE_MAX 1300  // 200 steps per rev, with 10 microsteps per step.

int serialOutput=0; // By default serial output of data is off, unti
// 'start' command is issued by computer.
                      
String readString;
int speed = 500;
int acc = 1000;
int homed = 0;
int direction = 1;
int running = 0; 
int ls1_triggered = 0;
int ls2_triggered = 0;
int targetPos = 0;

int cycle_min = 0;
int cycle_max = 1300;

int opMode = 0;   // 0=stop, 1=moveTo, 2=cycle

AccelStepper stepper;


// Function prototypes
void enableLsInt();
void disableLsInt();

int parseCmd(String cmdLine, String *key,String *value) {
  int equalsPos;
  equalsPos = cmdLine.indexOf('=');
  if (equalsPos==-1) {
    *key=cmdLine;
    *value="";
  }
  else {
    *key=cmdLine.substring(0,equalsPos);
    *value=cmdLine.substring(equalsPos+1);
  }
  return(equalsPos);
}

void setDirection(int d) {
  direction = d;
  //if (running) start();
}

void moveTo(int p) {
  if (homed) {
    int curPos = stepper.currentPosition();
    targetPos = p;
    if (curPos<p)
      setDirection(DIR_REV);
    else
      setDirection(DIR_FWD);
    running = 1;
    //Serial.print("MoveTo From ");
    //Serial.print(curPos);
    //Serial.print(" to ");
    //Serial.println(targetPos);
    stepper.moveTo(targetPos);
  } else {
    Serial.println("moveto(): ERROR - Not Homed");
  }
}


void startCycle() {
  Serial.println("startCycle()");
  digitalWrite(DIRECTION_PIN,direction);
  digitalWrite(13, direction); 
  targetPos = cycle_min;
  opMode = MODE_CYCLE;
  running = 1;
  moveTo(targetPos);
}


/**
 * If we are have reached the target position, checks if we are
 * in MODE_CYCLE.
 * if we are it sets a new target position.   If not it sets the 
 * 'running' flag to 0 to show we have stopped
 */
void checkCycle() {
  if (stepper.distanceToGo() == 0) {
    if (opMode == MODE_CYCLE) {
      if (direction == DIR_FWD) {
	//Serial.print("checkCycle FWD - moving to ");
	//Serial.println(CYCLE_MAX);
	moveTo(cycle_max);
      } else {
	//Serial.print("checkCycle REV - moving to ");
	//Serial.println(CYCLE_MIN);
	moveTo(cycle_min);
      }
    } else {
      running = 0;
    }
  }
}


void home() {
  int ls1val, ls2val;
  Serial.println("home()");
  disableLsInt();
  //Serial.println("Ls Int Disabled");
  
  /*while(1) {
    ls1val = digitalRead(LS1_PIN);
    ls2val = digitalRead(LS2_PIN);
    Serial.print("LS1=");
    Serial.print(ls1val);
    Serial.print(",  LS2=");
    Serial.println(ls2val);
  }*/
  

  // Home SLOWLY
  stepper.setMaxSpeed(HOME_SPEED);
  stepper.setAcceleration(HOME_ACC);

  // Reverse until we make the limit switch.
  Serial.println("Rotating until we hit limit switch...");
  ls1val = digitalRead(LS1_PIN);
  while (!ls1val) {
    //Serial.print(".");
    stepper.move(-1);
    stepper.run();
    ls1val = digitalRead(LS1_PIN);
    if (ls1val==1) {
      Serial.println("Re-reading to check for glitches");
      ls1val = digitalRead(LS1_PIN);
    }
  }
  // Go forwards until the limit switch resets.
  Serial.println("Reversing until we release limit switch...");
  while (ls1val) {
    stepper.move(1);
    stepper.run();
    ls1val = digitalRead(LS1_PIN);
    if (ls1val==0) {
      Serial.println("Re-reading to check for glitches");
      ls1val = digitalRead(LS1_PIN);
    }
  }
  stepper.setCurrentPosition(0);
  homed = 1;
  Serial.println("Homed!");
  //enableLsInt();
  Serial.println("Returning from home()");
}


void stop() {
  Serial.println("stop()");
  stepper.stop();
  opMode = MODE_STOP;
  running = 0;
}

void setSpeed(int s) {
  speed = s;
  if (s<SPEED_MIN) {
    speed=SPEED_MIN;
    Serial.println("Set to Minimum Speed");
  }
  if (s>SPEED_MAX) {
    speed = SPEED_MAX;
    Serial.println("Set to Maximum Speed");
  }
  stepper.setMaxSpeed(speed); 
}

void setAcc(int a) {
  acc = a;
  if (a<ACC_MIN) {
    acc=ACC_MIN;
    Serial.println("Set to Minimum Acc");
  }
  if (a>ACC_MAX) {
    a = ACC_MAX;
    Serial.println("Set to Maximum Acc");
  }
  stepper.setAcceleration(acc); 
}


void setCycleMax(int p) {
  cycle_max = p;
}

void setCycleMin(int p) {
  cycle_min = p;
}


void reverse() {
  int d;
  d = not(direction);
  setDirection(d);
}



void ls1_isr() {
  disableLsInt();
  int lsVal;
  // Crude attempt at debounce and to miss noise spikes.
  //delayMicroseconds(100);
  lsVal = digitalRead(LS1_PIN);
  //lsVal = digitalRead(LS1_PIN);
  //lsVal = digitalRead(LS1_PIN);
  if (!lsVal) {
    //stop();
    ls1_triggered = 1;
  }
  enableLsInt();
}

void ls2_isr() {
  disableLsInt();
  int lsVal;
  // Crude attempt at debounce and to miss noise spikes.
  //delayMicroseconds(100);
  lsVal = digitalRead(LS2_PIN);
  //lsVal = digitalRead(LS2_PIN);
  //lsVal = digitalRead(LS2_PIN);
  if (!lsVal) {
    //stop();
    ls2_triggered = 1;
  }
  enableLsInt();
}

void enableLsInt() {
  attachInterrupt(INT0, ls1_isr, FALLING);
  attachInterrupt(INT1, ls2_isr, FALLING);
  //attachInterrupt(digitalPinToInterrupt(LS1_PIN), ls1_isr, FALLING);
  //attachInterrupt(digitalPinToInterrupt(LS2_PIN), ls2_isr, FALLING);
}

void disableLsInt() {
  detachInterrupt(INT0);
  detachInterrupt(INT1);
  //detachInterrupt(digitalPinToInterrupt(LS1_PIN));
  //detachInterrupt(digitalPinToInterrupt(LS2_PIN));
}

void setup(){
  //start serial connection
  Serial.begin(9600);
  //configure pin2 as an input and enable the internal pull-up resistor
  pinMode(LS1_PIN, INPUT_PULLUP);
  pinMode(LS2_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(13, OUTPUT); 
  
  
  
  stepper = AccelStepper(AccelStepper::DRIVER,
			 STEP_PIN, 
			 DIRECTION_PIN);
  

  home();
  //startCycle();
  
  setSpeed(SPEED_INIT);
  setAcc(ACC_INIT);  
}




void loop_test() {
  if (stepper.distanceToGo() == 0)
    stepper.moveTo(-stepper.currentPosition());
  stepper.run();
}

void loop() {
  String k,v;
  //Serial.print(".");
  ////////////////////////////////////////////////
  // respond to commands from serial.
  ////////////////////////////////////////////////
  //readString="";
  while (Serial.available()) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }
  
  //if (readString.length()>0) {
  if(readString[readString.length()-1]=='\n') {
    Serial.println (readString);
    parseCmd(readString, &k,&v);
    Serial.print("parseCmd k=");
    Serial.print(k);
    Serial.print(". v=");
    Serial.print(v);
    Serial.println(".");
    
    // If no value specified we return the parameter value
    //
    if (v=="") {
      //Serial.println("v is empty");
      if (k=="speed\n") {
        Serial.print("speed=");
        Serial.println(speed);
      }
      if (k=="acc\n") {
        Serial.print("acc=");
        Serial.println(acc);
      }
      if (k=="start\n") {
        startCycle();
      }
      if (k=="stop\n") {
        stop();
      }
      if (k=="home\n") {
        Serial.println("Homing....");
        home();
      }
      if (k=="settings\n") {
        Serial.print("Speed,");
        Serial.print(speed);
        Serial.print(",");
        Serial.print("running");
        Serial.print(",");
        Serial.print(running);
      }
      
    }
    
    else {
      //Serial.println("V is not empty");
      if (k=="speed") {    //change speed
        Serial.print("Setting Speed=");
        Serial.println(v);
        setSpeed(v.toInt());
      }
      if (k=="acc") {    //change speed
        Serial.print("Setting acc=");
        Serial.println(v);
        setAcc(v.toInt());
      }
      if (k=="cmax") {    //change the maximum position during cycling
        Serial.print("Setting cmax=");
        Serial.println(v);
        setCycleMax(v.toInt());
      }
      if (k=="cmin") {    //change the maximum position during cycling
        Serial.print("Setting cmin=");
        Serial.println(v);
        setCycleMin(v.toInt());
      }
      if (k=="moveto") {    //change the maximum position during cycling
        Serial.print("moveto=");
        Serial.println(v);
        moveTo(v.toInt());
      }
      
    }
    
    readString = "";
    
  }
  
  
  if (serialOutput==1){
    Serial.print("data,");
    Serial.print(",");
  }
  
  //Serial.print(digitalRead(LS1_PIN));
  //Serial.print(", ");
  //Serial.println(digitalRead(LS2_PIN));
  
  if (ls1_triggered) {
    Serial.println("LS1 Triggered");
    stop();
    ls1_triggered = 0;
  }
  
  if (ls2_triggered) {
    Serial.println("LS2 Triggered");
    stop();
    ls2_triggered = 0;
  }

  // Check to see if we have reached the end of the cycle travel,
  // and reverse travel if necessary.
  stepper.run();
  checkCycle();
  
  //delay(DT* 1000);
  
}
