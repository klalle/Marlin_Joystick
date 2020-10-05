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

const int ledPin_01 = 10; // Analog output pin that the LED is attached to
const int ledPin_1 = 11;
const int ledPin_10 = 12;
const int ledPin_100 = 13;
const int btnActivated = 9; //side button for activate unit or not (default = high=>no btn)

int x=0; //initialize variables
int y=0;
int t=0;
char X_command[10] = "\0";
char Y_command[10]  = "\0";
char Z_command[10]  = "\0";
double XY_distance = 1.0;
double Z_distance = 1.0;

int XY_Feedrate = 4000;
int Z_Feedrate = 300;

int speedToggle=1; // set 0.1mm,1, 10, 100
bool activationBtnStatus = false;

unsigned long timeoutWaitingForOK = 250;
unsigned long longPressLimit = 1500;

bool joystickIsActivated=false;
bool updateSpeedAndLabel=true;

unsigned long minTimeToReleaseBtnDelay = 300;

bool firstXMoveLeft=true;
bool firstXMoveRight=true;

bool waitForOkResponse();
bool isPressed(int pin);
bool isLongPressed(int pin);
int getAverage(int pin);
void setLedPin(int pin);


void setup() {

  pinMode(PIN_BUTTON_A, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_C, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_D, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_E, INPUT_PULLUP);  
  pinMode(PIN_BUTTON_F, INPUT_PULLUP);
  pinMode(PIN_BUTTON_J, INPUT_PULLUP);
  pinMode(btnActivated, INPUT_PULLUP);

  pinMode(ledPin_01, OUTPUT);
  pinMode(ledPin_1, OUTPUT);
  pinMode(ledPin_10, OUTPUT);
  pinMode(ledPin_100, OUTPUT);

  setLedPin(-1); //No led on

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
      setLedPin(-1);
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
    if(!joystickIsActivated){
      Serial.println("M117 Long-click joystick!");
      setLedPin(-1);
    }
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
    setLedPin(speedToggle);

    switch(speedToggle) {
      case 0:
        XY_distance=0.1; 
        break;
      case 1: 
        XY_distance=1;
        break;  
      case 2: 
        XY_distance=10; 
        break;  
      case 3: 
          XY_distance=100;
          Z_distance = 20;// maximum Z-axis movement is 20mm not 100. We don't want to shoot it out of the MPCNC!
        break;  
    }
    if(speedToggle<3)
      Z_distance=XY_distance;

    Serial.print("M117 Joystick XY:");
    Serial.print(XY_distance);
    Serial.print(" Z:");
    Serial.println(Z_distance);
   
    //wait till btns released...
    while(isPressed(home_xy)){delay(50);} 
    while(isPressed(home_z)){delay(50);}
    while(isPressed(speedToggleBtn)){delay(50);}
    while(isPressed(joystickBtn)){delay(50);}
  }

  char strBuff_distance[10];

  //Keep moving in Z until jystick is released, start with long delay to enable single XY_distance moove
  //After that accelerate upp to speed.
  bool keepMoving=true; 
  bool first = true;
  bool hasMoved=false;

  unsigned long Z_moveTime = Z_distance/(Z_Feedrate/60)*1000;

  double keepMoving_distance=Z_distance;
  int decimals = 0;
  if(XY_distance<1)
    decimals=1;
  dtostrf(keepMoving_distance, 1, decimals, strBuff_distance);//first 1 is minimum length, second nr of decimals

  unsigned long travelingDuration=Z_moveTime;
  

  while(keepMoving){
    bool up_btn = isPressed(PIN_BUTTON_A);
    bool down_btn = isPressed(PIN_BUTTON_C);

    bool zMoved=true;
    if(up_btn)
      strcpy(Z_command, " Z");
    else if(down_btn)
      strcpy(Z_command, " Z-");
    else
      zMoved=false;

    if(zMoved){
      strcat(Z_command, strBuff_distance);

      if(first){
        Serial.println("G91"); //Set relative positioning
        hasMoved=true;
      }

      if(waitForOkResponse()){
        Serial.print("G1");
        Serial.print(Z_command);
        Serial.print(" F");
        Serial.println(Z_Feedrate);
        if(first){
          first = false;
          if(minTimeToReleaseBtnDelay<Z_moveTime)
            delay(Z_moveTime);
          else
            delay(minTimeToReleaseBtnDelay);
          
          keepMoving_distance=1;
          dtostrf(keepMoving_distance, 1, 0, strBuff_distance);
          travelingDuration=keepMoving_distance/(Z_Feedrate/60)*1000;
        }
        else
          delay(travelingDuration);
      }
      else
        first = false; //if no contact - do not spam with G91...
    }
    else
      keepMoving=false;
  }

  //Keep moving in XY until jystick is released, start with long delay to enable single XY_distance mooves
  //After that accelerate upp to speed.
  keepMoving=true; 
  first = true;

  keepMoving_distance=XY_distance;

  decimals=0;
  if(XY_distance<1)
    decimals=1;
  dtostrf(keepMoving_distance, 1, decimals, strBuff_distance);//first 1 is minimum length, second nr of decimals

  travelingDuration=0; //Sets in loop...
  keepMoving=true; 

  first = true;
  bool XYMove=false;


  while(keepMoving){
    //X
    bool xMoved=true;
    x = analogRead(x_Pin); 
    if (x < zeroState_x-treshold_x)
      strcpy(X_command," X-");
    else if (x > zeroState_x+treshold_x)
      strcpy(X_command," X");
    else
      xMoved=false;

    if(xMoved)
      strcat(X_command, strBuff_distance);

    //Y
    bool yMoved=true;
    y = analogRead(y_Pin);
    if (y < zeroState_y-treshold_y)
      strcpy(Y_command," Y-");
    else if (y > zeroState_y+treshold_y)
      strcpy(Y_command," Y");
    else
      yMoved=false;

    if(yMoved)
      strcat(Y_command, strBuff_distance);

    //Move!
    if(xMoved || yMoved){

      if(first && !hasMoved){
        Serial.println("G91"); //Set relative positioning
        hasMoved=true;
      }

      if(XYMove != (xMoved && yMoved) || first){
        XYMove = (xMoved && yMoved);
        if(XYMove) //Diagonal move = longer XY_distance... X=5, Y=5 => H=sqrt(2)*X (Pythagoras) H=sqrt(2*X²)=sqrt(2)*X
          travelingDuration=(1.41421*keepMoving_distance)/(XY_Feedrate/60)*1000; 
        else
          travelingDuration=keepMoving_distance/(XY_Feedrate/60)*1000; 
      }

      if(waitForOkResponse()){
        Serial.print("G1");
        if(xMoved)
          Serial.print(X_command);
        if(yMoved)
          Serial.print(Y_command);
        Serial.print(" F");
        Serial.println(XY_Feedrate);

        if(first){
          first = false;
          if(minTimeToReleaseBtnDelay<travelingDuration)
            delay(travelingDuration);
          else
            delay(minTimeToReleaseBtnDelay);

          keepMoving_distance=5;
          dtostrf(keepMoving_distance, 1, 0, strBuff_distance);
          if(XYMove) //Diagonal move = longer XY_distance
            travelingDuration=(1.41421*keepMoving_distance)/(XY_Feedrate/60)*1000; 
          else
            travelingDuration=keepMoving_distance/(XY_Feedrate/60)*1000; 
        }
        else
          delay(travelingDuration);
      }
      else
        first = false;
    }
    else
      keepMoving=false;
  }
  if(hasMoved){
    Serial.println("G90"); //Set back to absolute positioning (gcode from sd)
  }
}

bool waitForOkResponse(){
  unsigned long startedWaiting = millis();
  String receivedStr = "";
  bool OkFound=false;
  while (Serial.available() && millis() - startedWaiting <= minTimeToReleaseBtnDelay) {
    char inChar = (char)Serial.read();
    receivedStr += inChar;
    if(receivedStr.indexOf("ok")>=0)
      OkFound=true;
  }
  return OkFound;
}

void setLedPin(int pin){
  digitalWrite(ledPin_01, HIGH); //turn all off
  digitalWrite(ledPin_1, HIGH);
  digitalWrite(ledPin_10, HIGH);
  digitalWrite(ledPin_100, HIGH);
  switch (pin) //Turn on one or none
  {
    case 0:
      digitalWrite(ledPin_01, LOW);
      break;
    case 1:
      digitalWrite(ledPin_1, LOW);
      break;
    case 2:
      digitalWrite(ledPin_10, LOW);
      break;
    case 3:
      digitalWrite(ledPin_100, LOW);
      break;
    default:
      // eg -1 leaves all leds off
      break;
  }
}

bool isPressed(int pin){
  return digitalRead(pin)==LOW;
}

bool isLongPressed(int pin){
  int c=0;
    unsigned long startedWaiting = millis();
    while(digitalRead(pin)==LOW && millis() - startedWaiting <= longPressLimit){
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