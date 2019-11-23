
#define LS1_PIN 2
#define LS2_PIN 3
#define STEP_PIN 4
#define DIRECTION_PIN 5

#define DT 1.0   // Interface polling period (Sec)

#define SPEED_MIN 31    // Limit of arduino tone library
#define SPEED_MAX 10000  // Motor doesn't like being stepped any faster.

int serialOutput=0; // By default serial output of data is off, unti
                      // 'start' command is issued by computer.
                      
String readString;
int speed = 2000;
int direction = 1;
int running = 0;
int ls1_triggered = 0;
int ls2_triggered = 0;

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






void setup(){
  //start serial connection
  Serial.begin(9600);
  //configure pin2 as an input and enable the internal pull-up resistor
  pinMode(LS1_PIN, INPUT_PULLUP);
  pinMode(LS2_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(13, OUTPUT); 
  
  //attachInterrupt(digitalPinToInterrupt(LS1_PIN), ls1_isr, RISING)
  attachInterrupt(INT0, ls1_isr, RISING);
  attachInterrupt(INT1, ls2_isr, RISING);
  
}


void start() {
  digitalWrite(DIRECTION_PIN,direction);
  digitalWrite(13, direction); 
  tone(STEP_PIN, speed);
  running = 1;
}

void stop() {
   noTone(STEP_PIN);
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
  if (running) start(); 
}


void setDirection(int d) {
  direction = d;
  if (running) start();
}

void reverse() {
  int d;
  d = not(direction);
  setDirection(d);
}


void loop() {
  String k,v;
  ////////////////////////////////////////////////
  // respond to commands from serial.
  ////////////////////////////////////////////////
  while (Serial.available()) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      readString += c;
    }
  }

  //Serial.println (readString);
  if (readString.length()>0) {
      parseCmd(readString, &k,&v);
      Serial.print("parseCmd k=");
      Serial.print(k);
      Serial.print(". v=");
      Serial.print(v);
      Serial.println(".");
    
  // If no value specified we return the parameter value
  //
  if (v=="") {
    Serial.println("v is empty");
      if (k=="speed") {
        Serial.print("speed=");
        Serial.println(speed);
      }
      if (k=="start\n") {
        start();
      }
      if (k=="stop\n") {
        stop();
      }
      if (k=="reverse\n") {
        Serial.println("Reversing");
        reverse();
      }
      if (k=="settings") {
        Serial.print("Speed,");
        Serial.print(speed);
        Serial.print(",");
        Serial.print("running");
        Serial.print(",");
        Serial.print(running);
      }

    }
    
    else {
      Serial.println("V is not empty");
      if (k=="speed") {    //change speed
        Serial.print("Setting Speed to ");
        Serial.println(v);
        setSpeed(v.toInt());
      }

    }
    
    readString = "";

    }
  

    if (serialOutput==1){
      Serial.print("data,");
      Serial.print(",");
    }
  
  Serial.print(digitalRead(LS1_PIN));
  Serial.print(", ");
  Serial.println(digitalRead(LS2_PIN));
  
  if (ls1_triggered) {
    Serial.println("LS1 Triggered");
    ls1_triggered = 0;
  }
  if (ls2_triggered) {
    Serial.println("LS2 Triggered");
    ls2_triggered = 0;
  }
  delay(DT* 1000);

}

void ls1_isr() {
  stop();
  ls1_triggered = 1;
}

void ls2_isr() {
  stop();
  ls2_triggered = 1;
}



