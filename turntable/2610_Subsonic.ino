#include <AccelStepper.h>

//setup stepper
AccelStepper stepper(AccelStepper::DRIVER, 2,1);

//parsing vars
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

//initialize  various vars
String angle1 = "a";
String done = "Running";
String alarm = "c"; 
float angleprint = 0; 
int accel = 0;
float pos = 0;
int vel = 30000;
int bounces = 0; 
float stepsrev = 0;
float stepsrevrecip = 0;
int bouncecnt = 0;  
float angle = 0;
boolean reset = false; 
boolean estop = false;
boolean zero = false; 
boolean bounce = false; 
int maxaccel = 2000; 
int maxvel = 4000; 
int maxpos = 10000; 
int pw = 0;
int pwver = 1885;
bool construn = false; 
bool slowstop = false; 
unsigned long t = 0; 
String t_t = "d"; 
boolean newData = false;
int delayMillis = 5000;
unsigned long timeNow = 0;
float traverseIncrement = 111.11111;
float endpos = 20;
float startpos = 0; 
boolean LH = false; 


void setup() {
  Serial.begin(115200);
  stepper.setMaxSpeed(40000);
  stepper.setAcceleration(12000);
  // Set up interrupts on limit switch inputs
  pinMode(4, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(4), stop, FALLING);
  pinMode(5, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(5), stop, FALLING);
}

void loop() {

  //read data from one time command
  if (Serial.available() > 0)  {
        recvWithStartEndMarkers(); // recieve serial data
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
  parseData(); 
  
  //recognize reset command
  if (bounces == 30001){
    reset = true;  
  }
  
  //recognize estop command
  if (bounces == 30002){
    estop = true;
  }
  
  //recognize zero command
  if (bounces == 30003) {
   zero = true;
  }
  
  //recognize command to go into constant run mode
  if (bounces == 30004) {
    construn = true; 
  }
  
  //recognize command for a slow stop
  if (bounces == 30005) {
    slowstop = true; 
  }

  //recognize traverse command
 if (bounces == 30006) {
  traverseFunc(); 
  bounces = 0; 
 }
 
  //set bounce limit 
  if ((bounces == 0) || (bounces > 1000)) {
     bounce = false; 
     bouncecnt = 0; 
  }
  else if (bounces < 1000) {
    bounce = true; 
  }
  
  //speed, accel, position limits
  if ((accel > maxaccel || vel > maxvel || pos > maxpos) && (pw != pwver)) {
    stepper.setMaxSpeed(0); 
    reset = true; 
    construn = false; 
  }

  // set parameters 
  pos = (angle/360) * stepsrev; 
  stepsrevrecip = 1 / stepsrev;
  stepper.moveTo(pos);
  stepper.setMaxSpeed(vel); //assign the rpm recieved over serial to the motor
  stepper.setAcceleration(accel);
  } 

  if ((bounce) && (bounces < 1000)) { //BOUNCE MODE
   bouncefunc(); 
  }
  
  else { //NORMAL RUNNING 
   
  //constant running
  while (construn == true) {
   stepper.setSpeed(vel); 
   stepper.runSpeed(); 

   angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
   angle1 = String(angleprint,0);
   t = (millis()); 
   Serial.println("<" + angle1 + ", " + done + "; " + t + ">");

   if (Serial.available() > 0)  {
        recvWithStartEndMarkers(); // recieve serial data
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
    parseData(); 

    if (bounces != 30004) {
      construn = false; 
    }

     if (bounces == 30001){
    reset = true;  
  }

  if (bounces == 30002){
    estop = true;
  }
  
  if (bounces == 30003) {
   zero = true;
  }

  if (bounces == 30004) {
    construn = true; 
  }

  if (bounces == 30005) {
    slowstop = true; 
  }
   } 
  }
     
    // reset to zero when running or stopped
  if (reset) {
  stepper.setAcceleration(5000); 
  stepper.stop();
  stepper.moveTo(0);
  stepper.setMaxSpeed(1000);
  
  while (stepper.distanceToGo() != 0) {
    stepper.run(); 
    angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
   angle1 = String(angleprint,0);
   t = (millis()); 
   Serial.println("<" + angle1 + ", " + "RESETTING" + "; " + t + ">");
  }
  reset = false; 
 
  }

 //reset when a limit is hit
  if (LH) {
  stepper.setAcceleration(5000); 
  stepper.stop();
  stepper.moveTo(0);
  stepper.setMaxSpeed(1000);
  
  while (stepper.distanceToGo() != 0) {
    stepper.run(); 
    angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
   angle1 = String(angleprint,0);
   t = (millis()); 
   Serial.println("<" + angle1 + ", " + "LimitHOMING" + "; " + t + ">");
  }
  LH = false; 
 
  }
  

  //normal stop
  if (slowstop) {
    stepper.stop();
    slowstop = false; 
  }
  
  //emergency stop (stop and don't reset)
  if (estop) {
    stepper.setAcceleration(10000);
    construn = false; 
    stepper.stop(); 
    estop = false; 
    Serial.println("Stopping");
  }
 
  //zero 
  if (zero) {
  stepper.setCurrentPosition(0);  
  zero = false;  
  }
   
   //run the motor 
   stepper.run(); 

  if (stepper.run()) {
    done = "Running"; 
  }
   else {
    done = "Complete"; 
   }
   
   angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
   angle1 = String(angleprint,0);
   t = (millis()); 
   Serial.println("<" + angle1 + ", " + done + "; " + t + ">");
  }
  }

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    while (Serial.available() > 0) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    accel = atoi(strtokIndx);
 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    angle = atof(strtokIndx);     // convert this part to a float
    
    strtokIndx = strtok(NULL, ",");
    vel = atoi(strtokIndx);     // convert this part to an int

    strtokIndx = strtok(NULL, ",");
    bounces = atoi(strtokIndx);     // convert this part to an int

    strtokIndx = strtok(NULL, ",");
    stepsrev = atoi(strtokIndx);     // convert this part to an int

    strtokIndx = strtok(NULL, ",");
    pw = atoi(strtokIndx);     // convert this part to an int
}

void bouncefunc() { 

if (LH) {
  stepper.setAcceleration(5000); 
  stepper.stop();
  stepper.moveTo(0);
  stepper.setMaxSpeed(1000);
  
  while (stepper.distanceToGo() != 0) {
    stepper.run(); 
    angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
   angle1 = String(angleprint,0);
   t = (millis()); 
   Serial.println("<" + angle1 + ", " + "LimitHOMING" + "; " + t + ">");
  }
  LH = false; 
 
  }
   
   // reset to zero when running or stopped   
  if (reset) {
  stepper.setAcceleration(5000); 
  stepper.stop(); 
  stepper.moveTo(0);
  stepper.setMaxSpeed(1000);
  
  while (stepper.distanceToGo() != 0) {
    stepper.run(); 
  }
  reset = false; 
  Serial.println("Resetting"); 
  }

  //emergency stop (stop and don't reset)
  if (estop) {
    stepper.setAcceleration(10000);
    stepper.stop(); 
    estop = false; 
    Serial.println("Stopping");
  }
 
  //zero 
  if (zero) {
  stepper.setCurrentPosition(0);  
  Serial.println("Zeroing");
  zero = false;  
  }


   // If at the end of travel go to the other end
    if ((stepper.distanceToGo() == 0) && (bouncecnt < bounces) && (bounces < 1000)) {
      bouncecnt++; 
      stepper.moveTo(-stepper.currentPosition());}
   
    stepper.run();
   
   if (bouncecnt >= bounces && bounce == true) {
    stepper.stop();
    bounces = 0; 
    bounce = false; 
    bouncecnt = 0; 
    delay(1000);
    reset = true; 
   }
   
  if (stepper.run()) {
    done = "Running"; 
  }
   else {
    done = "Complete"; 
   }
   
   angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
   angle1 = String(angleprint,0);
   t = (millis()); 
   Serial.println("<" + angle1 + ", " + done + "; " + t + ">" + bouncecnt);
  }

  void traverseFunc() {
 
    timeNow = millis();
    while (millis() - timeNow < delayMillis){
      Serial.println("<" + String(((float(stepper.currentPosition()*1000) - 4000))*.009009) + ", LOGGING; " + millis() + ">");
    }
  
  // Execute traverse profile
  for (float i = 1.0; (i < (endpos + abs(startpos) + 1)) && (LH == false) ; i++){
    stepper.moveTo(float(stepper.currentPosition()) + traverseIncrement);
    while (stepper.distanceToGo() != 0){
      stepper.run();
      Serial.println("<" + String(((float(stepper.currentPosition()*1000) - 4000))*.009009) + ", RUNNING; " + millis() + ">");
    }
    timeNow = millis();
    while (millis() - timeNow < delayMillis){
      Serial.println("<" + String(((float(stepper.currentPosition()*1000) - 4000))*.009009) + ", LOGGING; " + millis() + ">");
    } 
  }
  
if (LH) {
      
  stepper.setAcceleration(5000); 
  stepper.stop();
  stepper.moveTo(0);
  stepper.setMaxSpeed(1000);
  
  while (stepper.distanceToGo() != 0) {
      stepper.run(); 
      angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
      angle1 = String(angleprint,0);
      t = (millis()); 
      Serial.println("<" + angle1 + ", " + "LimitHOMING" + "; " + t + ">");
  }
  
  
  }
 
  // Return home and clear buffer
  stepper.moveTo(0);
  while (abs(stepper.distanceToGo()) > 0){
      stepper.run();
      Serial.println("<" + String(((float(stepper.currentPosition()*1000) - 4000))*.009009) + ", HOMING; " + millis() + ">");
  }
  Serial.read(); 
  //Serial.flush(); 
  
  }


  
void stop() {
  LH = true; 
}

//void limithome() {
//  bounces = 0; 
//  stepper.setAcceleration(5000); 
//  stepper.stop();
//  stepper.moveTo(0);
//  stepper.setMaxSpeed(1000);
//  
//  while (stepper.distanceToGo() > 0) {
//    stepper.run(); 
//    angleprint = int(stepper.currentPosition()) * (360000 * stepsrevrecip); 
//   angle1 = String(angleprint,0);
//   t = (millis()); 
//   Serial.println("<" + angle1 + ", " + "LIMITHOMING" + "; " + t + ">");
//  }
//  reset = false; 
//  bounces = 0; 
//} 
