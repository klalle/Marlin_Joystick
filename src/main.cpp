#include <Arduino.h>
// #include <SoftwareSerial.h>

// These constants won't change. They're used to give names to the pins used:
// user configurable options
const int treshold_x = 50; // increase this value, if x-axis of joystick is not giving zero values when idle. Keep values between 300 and 0. Decrease to get joystick act more sensible.
const int treshold_y = 50; // increase this value, if y-axis of joystick is not giving zero values when idle. Keep values between 300 and 0. Decrease to get joystick act more sensible.
int zeroState_x =510;
int zeroState_y=510;

const long baud_rate=250000; // Baudrate of your MPCNC-Controller

const int x_Pin = A0;  // Analog input pin that the x-potentiometer is attached to
const int y_Pin = A1;  // Analog input pin that the y-potentiometer is attached to
const byte PIN_BUTTON_A = 2; //Z+
const byte PIN_BUTTON_B = 3; //Go to home (X,Y)
const byte PIN_BUTTON_C = 4; //Z-
const byte PIN_BUTTON_D = 5; //Home X,Y (endstops)

const byte PIN_BUTTON_E = 6; //Home z (endstops)
const byte PIN_BUTTON_F = 7; //Speed toggle

const byte PIN_BUTTON_J = 8; //(joystickBtn) 

const int home_xy=PIN_BUTTON_D; 
const int home_z=PIN_BUTTON_E; 
const int setHomeHere=PIN_BUTTON_B; 
const int joystickBtn=PIN_BUTTON_J;
const int speedToggleBtn = PIN_BUTTON_F; 

boolean hxy = false; //define buttons_presed variables
boolean hz = false;

const int ledPin_01 = 9; // Analog output pin that the LED is attached to
const int ledPin_1 = 10;
const int ledPin_10 = 11;
const int ledPin_100 = 12;
const int btnActivated = 13; //side button for activate unit or not (default = high=>no btn)

int x=0; //initialize variables
int y=0;
int t=0;
String X_command = "";
String Y_command = "";
String Z_command = "";
String distance = "1.0 ";
String Z_distance = "1.0 ";

int XY_Feedrate = 2000;
int Z_Feedrate = 300;

int speedToggle=1; // set 0.1mm,1, 10, 100
bool activationBtnStatus = false;

//String inputString = "";         // a string to hold incoming data
//boolean stringComplete = false;  // whether the string is complete

bool joystickIsActivated=false;
bool updateSpeedAndLabel=true;

int minTimeToReleaseBtnDelay = 500;
int XY_moveTime=0;
int Z_moveTime=0;

bool firstXMoveLeft=true;
bool firstXMoveRight=true;

bool isPressed(int pin);
bool isLongPressed(int pin);
int getAverage(int pin);

void setup() {

  pinMode(PIN_BUTTON_A, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_C, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_D, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_E, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_F, INPUT_PULLUP);
  pinMode(PIN_BUTTON_J, INPUT_PULLUP);
  pinMode(btnActivated, INPUT_PULLUP);

  Serial.begin(baud_rate);
  
  delay(3000);
  zeroState_x = getAverage(x_Pin);
  zeroState_y = getAverage(y_Pin);
  Serial.println("M117 Long-click joystick!");
}



void loop() {
  if(activationBtnStatus && !isPressed(btnActivated)){ //just turned btn off
    activationBtnStatus=false;
    Serial.println("M117 Long-click joystick!");
  }
  else if(!activationBtnStatus && isPressed(btnActivated)) //just turned btn on
  {
    activationBtnStatus=true;
    updateSpeedAndLabel=true;
    joystickIsActivated=false;
    speedToggle=1;
  }
  else if(!activationBtnStatus && isLongPressed(joystickBtn)){ //btn=off, but using joystick to toggle on/off
    joystickIsActivated=!joystickIsActivated;
    if(!joystickIsActivated)
      Serial.println("M117 Long-click joystick!");
    else{
      updateSpeedAndLabel=true;
      speedToggle=1;
    }
  }

  if(!joystickIsActivated && !activationBtnStatus){
    //TODO set led-pin
    delay(20);
    return;
  }else{
    //TODO set led-pin
  }
  if (isLongPressed(home_xy))
  {
    Serial.println("G28 X Y");
    Serial.println("M117 Homing XY...");
    updateSpeedAndLabel=true;
    delay(3000);
  }

  if (isLongPressed(home_z))
  {
    Serial.println("G28 Z");
    Serial.println("M117 Homing Z...");
    updateSpeedAndLabel=true;
    delay(3000);
  }

  if (isLongPressed(setHomeHere))
  {
    Serial.println("G92 X0 Y0 Z0");
    Serial.println("M117 Home is reset!");
    updateSpeedAndLabel=true;
    delay(3000);
  }
  if(isPressed(speedToggleBtn)) //F-btn
  {
    speedToggle+=1;
    if(speedToggle>3)
      speedToggle=0;
    updateSpeedAndLabel=true;
    delay(50);
  }


  if(updateSpeedAndLabel){
    updateSpeedAndLabel=false;
    switch(speedToggle) {
      case 0:
        distance="0.1"; //set movement to 0.1mm
        XY_moveTime=0.1/(XY_Feedrate/60)*1000+10; //+10 => Marlin get clogged with commands...
        Z_moveTime =0.1/(Z_Feedrate/60)*1000+10;
        //TODO set led-pins
        break;
      case 1: 
        distance="1"; //set movement to 1mm
        XY_moveTime=1.0/(XY_Feedrate/60)*1000+10;
        Z_moveTime =1.0/(Z_Feedrate/60)*1000+10;
        //TODO set led-pins
        break;  
      case 2: 
        distance="10"; //set movement to 10mm
        XY_moveTime=10.0/(XY_Feedrate/60)*1000;
        Z_moveTime =10.0/(Z_Feedrate/60)*1000;
        //TODO set led-pins
        break;  
      case 3: 
          Z_distance = "20";// maximum Z-axis movement is 20mm not 100. We don't want to shoot it out of the MPCNC!
          Z_moveTime = 20.0/(Z_Feedrate/60)*1000;
          distance="100"; //set movement to 50
          XY_moveTime=100.0/(XY_Feedrate/60.0)*1000; //100mm, / (F2000[mm/min]/60[s/min])*1000[ms/s]
          //TODO set led-pins
        break;  
    }
    if(speedToggle<3)
      Z_distance=distance;

    Serial.print("M117 Joystick XY:");
    Serial.print(distance);
    Serial.print(" Z:");
    Serial.println(Z_distance);
   
    //wait till btns released...
    while(isPressed(home_xy)){delay(50);} 
    while(isPressed(home_z)){delay(50);}
    while(isPressed(speedToggleBtn)){delay(50);}
    while(isPressed(joystickBtn)){delay(50);}
  }

  //Keep moving in Z until jystick is released, start with long delay to enable single distance moove
  //After that accelerate upp to speed.
  bool keepMoving=true; 
  bool first = true;

  int keepMovingFeedrate=Z_Feedrate;
  String keepMovingDistance=Z_distance;
  int keepMovingMoveTime=Z_moveTime;//1.0/(Z_Feedrate/60)*1000+10;

  while(keepMoving){
    Z_command =" Z0";
    bool up_btn = isPressed(PIN_BUTTON_A);
    bool down_btn = isPressed(PIN_BUTTON_C);
      
    if(up_btn)
      Z_command = String(" Z" + keepMovingDistance);
      
    if(down_btn)
      Z_command = String(" Z-" + keepMovingDistance);

    if(Z_command !=" Z0"){
      Serial.println("G91"); //Set relative positioning
      Serial.print("G1");
      Serial.print(Z_command);
      Serial.print(" F");
      Serial.println(keepMovingFeedrate);
      Serial.println("G90"); //Reset Absolute positioning (used in gcode)

      if(first){
        first = false;
        if(minTimeToReleaseBtnDelay<Z_moveTime){
          delay(Z_moveTime);
          keepMovingMoveTime=0.5/(keepMovingFeedrate/60)*1000;
        }
        else{
          delay(minTimeToReleaseBtnDelay);
          keepMovingFeedrate = 125;
        }
        keepMovingDistance="0.5";
      }
      else
        delay(keepMovingMoveTime);
    }
    else
      keepMoving=false;

    if(keepMovingFeedrate+25<=Z_Feedrate){ //accelerate up to speed...
      keepMovingFeedrate+=25;
      keepMovingMoveTime=0.5/(keepMovingFeedrate/60)*1000;
    }
  }

  //Keep moving in XY until jystick is released, start with long delay to enable single distance mooves
  //After that accelerate upp to speed.
  keepMoving=true; 
  first = true;

  keepMovingFeedrate=XY_Feedrate;
  keepMovingDistance=distance;
  keepMovingMoveTime=XY_moveTime;
  keepMoving=true; 

  first = true;
  while(keepMoving){
    X_command =" X0";
    Y_command =" Y0";
    x = analogRead(x_Pin); 
    if (x < zeroState_x-treshold_x)
      X_command = String(" X-" + keepMovingDistance);
    else if (x > zeroState_x+treshold_x)
      X_command = String(" X" + keepMovingDistance);

    y = analogRead(y_Pin);
    if (y < zeroState_y-treshold_y)
      Y_command = String(" Y-" + keepMovingDistance);
    else if (y > zeroState_y+treshold_y)
      Y_command = String(" Y" + keepMovingDistance);

    if(X_command !=" X0" || Y_command !=" Y0"){
      Serial.println("G91");
      Serial.print("G1");
      Serial.print(X_command);
      Serial.print(Y_command);
      Serial.print(" F");
      Serial.println(keepMovingFeedrate);
      Serial.println("G90");

      if(first){
        first = false;
        if(minTimeToReleaseBtnDelay<XY_moveTime){
          delay(XY_moveTime);
          keepMovingMoveTime=2.0/(keepMovingFeedrate/60)*1000;
        }
        else{
          delay(minTimeToReleaseBtnDelay);
          keepMovingFeedrate = 400;
        }
        keepMovingDistance="2";
      }
      else
        delay(keepMovingMoveTime);
    }
    else
      keepMoving=false;

    if(keepMovingFeedrate+100<=XY_Feedrate){ //accelerate up to speed...
      keepMovingFeedrate+=100;
      keepMovingMoveTime=2.0/(keepMovingFeedrate/60)*1000;
    }
  }
}

bool isPressed(int pin){
  return digitalRead(pin)==LOW;
}

bool isLongPressed(int pin){
  int c=0;
    unsigned long startedWaiting = millis();
    while(digitalRead(pin)==LOW && millis() - startedWaiting <= 1500){
      delay(10);
      c++;
    }
    if(c>150-1)
      return true;
    else
      return false;
}

int getAverage(int pin){
  int a =0;
  for(int i=0;i<10;i++){
    a=a+analogRead(pin);
    delay(20); 
  }
  return a/10;
}


//Serial event
// void serialEvent() {
//   while (Serial.available()) {
//     // get the new byte:
//     char inChar = (char)Serial.read();
//     // add it to the inputString:
//     inputString += inChar;
//     // if the incoming character is a newline, set a flag
//     // so the main loop can do something about it:
//     if (inChar == '\n') 
//       stringComplete = true;
//   }
// }