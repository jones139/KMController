
#define LS1_PIN 2
#define LS2_PIN 3
#define STEP_PIN 4
#define DIRECTION_PIN 5

#define DT 1.0

int serialOutput=0; // By default serial output of data is off, unti
                      // 'start' command is issued by computer.
                      
String readString;
int speed = 2000;
int direction = 1;
int running = 0;

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
        speed = v.toInt();
      }

    }
    
    readString = "";

    }
  

    if (serialOutput==1){
      Serial.print("data,");
      Serial.print(",");
    }
  
  delay(DT* 1000);

}



