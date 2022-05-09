//A Demo for Tuesday.
//Missing the Matrix until further notice because it crashed the arduino.

//Include Libraries
#include <Servo.h> //Servo Library
#include <CheapStepper.h> //Stepper Library
#include "pitches.h"

//Global Variables
Servo servoLid;  // Define Lid Servo
Servo servoArm;  // Define Arm Servo
CheapStepper stepper (A1,A2,A4,A5); // Define Stepper on pins A1, A2, A4, A5

int buzzerPin = A0;
int buttonState1, buttonState2, buttonState3, buttonState4, buttonState5; // Initliazing Button States
int zeroPosition; //Initializing zeroPosition

//BUZZER
void Win (){
  tone(buzzerPin,NOTE_E6,125);
  delay(130);
  tone(buzzerPin,NOTE_G6,125);
  delay(130);
  tone(buzzerPin,NOTE_E7,125);
  delay(130);
  tone(buzzerPin,NOTE_C7,125);
  delay(130);
  tone(buzzerPin,NOTE_D7,125);
  delay(130);
  tone(buzzerPin,NOTE_G7,125);
  delay(125);
  noTone(buzzerPin);
  }

  void Damage (){
    tone(buzzerPin,NOTE_C5);
    delay(200);
    tone(buzzerPin,NOTE_G3);
    delay(500);
    noTone(buzzerPin);
  }

  void NewLevel(){
  
  tone(buzzerPin,NOTE_F5);
  delay(200);
  noTone(buzzerPin);
  delay(200);
  tone(buzzerPin,NOTE_F6);
  delay(200);
  noTone(buzzerPin);
  delay(200);
  tone(buzzerPin,NOTE_A4);
  delay(200);
  noTone(buzzerPin);
  delay(200);
  tone(buzzerPin,NOTE_G7);
  delay(1000);
  noTone(buzzerPin);
  }
//BUZZER END

//PRE: Buttonstates initialised
//POST: Pinmodes 
void defineSwitchPins(){
  //Switch Pins, from left to right
  //Switch Pin 1
  pinMode(2, INPUT_PULLUP);
  //Switch Pin 2
  pinMode(4, INPUT_PULLUP);
  //Switch Pin 3
  pinMode(5, INPUT_PULLUP);
  //Switch Pin 4
  pinMode(7, INPUT_PULLUP);
  //Switch Pin 5
  pinMode(10, INPUT_PULLUP);
}

//PRE: Servos Initialized
//POST: ServoLid attaches to pin 6, ServoArm attaches to pin A3, they both go into their starting position.
void servoSetup(){
  Serial.println("Servo Setup..");
  //Attaches the Servo from the Lid to Pin 6
  servoLid.attach(6);
  //Attaches the Servo from the Arm to Pin A3
  servoArm.attach(A3);

  servoArm.write(180); //Hide Arm

  servoLid.write(172); //Close Lid
  }

//PRE: Assumes ~7V current, stepper is initialized
//POST: Stepper is ready to hit middle switch, zeroPosition is equal to fully to touching the right wall.
void stepperSetup(){
  Serial.println("Stepper Setup.."); 
  stepper.setTotalSteps(4076); // Motor is geared 63.68395:1 (measured) rather than 64:1 (advertised)
  stepperCalibration();
  
  //Move to Middle
  stepper.moveCCW (650);
  }

//PRE: Stepper initialised
//POST: zeroPosition is equal to fully to touching the right wall.
void stepperCalibration(){
  Serial.println("Stepper Calibration..");
  //Calibration
  stepper.setRpm(18); //18 is strong enough to move the contraption, but as soon as it hits the wall stepper doesn't move anymore
  stepper.moveCW(1500); //Stepper hits the right wall
  zeroPosition = stepper.getStep(); //We remember where the stepper is right now
  }

//PRE: buttonStates initialized
//POST: buttonStates are read and printed.
void readButtonStates(){
  Serial.println("Reading Button States..");
  buttonState1 = digitalRead(2); 
  buttonState2 = digitalRead(4);
  buttonState3 = digitalRead(5);
  buttonState4 = digitalRead(7);
  buttonState5 = digitalRead(10);
  Serial.print("buttonstate1 = "); Serial.print(buttonState1);
  Serial.println();
  Serial.print("buttonstate2 = "); Serial.print(buttonState2);
  Serial.println();
  Serial.print("buttonstate3 = "); Serial.print(buttonState3);
  Serial.println();
  Serial.print("buttonstate4 = "); Serial.print(buttonState4);
  Serial.println();
  Serial.print("buttonstate5 = "); Serial.print(buttonState5);
  Serial.println();
  
}

void moveArmToSwitch(int buttonNumber){
  Serial.print("Moving Arm to Switch: "); Serial.print(buttonNumber);Serial.println();
    //These two Variables are just for simplicity's sake. true -> stepper turns clockwise -> arm moves to the right.
    bool moveToRight = true;
    bool moveToLeft = false;
    bool moveDir = moveToLeft; //Last move in calibration is moving left

    //If Arm moves to wrong position, you can change the position here! If Arm is left of button, decrease the position value. If Arm is right of button, increase the position value.
    //Attention! If the arm comes from the right, steps are added to the position automatically, you can change it in the if statements below of the respective buttons.
    int button1Position = zeroPosition - 1200; 
    int button2Position = zeroPosition - 900;
    int button3Position = zeroPosition - 600;
    int button4Position = zeroPosition - 300;
    int button5Position = zeroPosition; 
    int buttonPosition;
    
    switch(buttonNumber){
        case 1:
          buttonPosition = button1Position;
          Serial.println("Buttonposition: "); Serial.print(buttonPosition);Serial.println();
          break;
        case 2:
          buttonPosition = button2Position;
          Serial.println("Buttonposition: "); Serial.print(buttonPosition);Serial.println();
          break;
        case 3:
          buttonPosition = button3Position;
          Serial.println("Buttonposition: "); Serial.print(buttonPosition);Serial.println();
          break;
        case 4:
          buttonPosition = button4Position;
          Serial.println("Buttonposition: "); Serial.print(buttonPosition);Serial.println();
          break;
        case 5:
          buttonPosition = button5Position;
          Serial.println("Buttonposition: "); Serial.print(buttonPosition);Serial.println();
          break;
      }
    
    if(stepper.getStep() < buttonPosition){
      //If Arm is to the left of Switch
        stepper.moveTo (moveToRight, buttonPosition);
        
        if(moveDir != moveToRight){
          //stepper.moveCW(50);
          }
        moveDir = moveToRight;
        }
      else{
        stepper.moveTo (moveToLeft, buttonPosition);
        if(moveDir != moveToRight){
          stepper.moveCCW(50);
          }
        moveDir = moveToLeft;
    }
    //Allowed to be outside because function is only called when button is actually pressed.
    pressButton();
  }

int checkIfButtonPressed(){
    Serial.println("Checking if Button is Pressed..");
    readButtonStates();
    int buttonPressed;
    if(buttonState1 == LOW){
      buttonPressed = 1;
    }else if(buttonState2 == LOW){
      buttonPressed = 2;
    }else if(buttonState3 == LOW){
      buttonPressed = 3;
    }else if(buttonState4 == LOW){
      buttonPressed = 4;
    }else if(buttonState5 == LOW){
      buttonPressed = 5;
    }else{
      buttonPressed = 0;
    }
    return buttonPressed;
}

void openLid(){
  servoLid.write(140);
  delay(50);
  }

void closeLid(){
  servoLid.write(172);
  delay(50);
  }

void openArm(){
  servoArm.write(110);
  delay(200);
  }

void closeArm(){
  servoArm.write(180);
  delay(50);
  }
  
void pressButton(){
  Serial.println("Pressing Button..");
  
  //Open Lid  
  openLid();
  
  //Press button
  openArm();

  //Hide Arm
  closeArm();

  
  
  //Close Lid
  closeLid();
}

void wheelSetup(){
  //Setup Channel A
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT); //Initiates Brake Channel A pin

  //Setup Channel B
  pinMode(13, OUTPUT); //Initiates Motor Channel A pin
  pinMode(8, OUTPUT);  //Initiates Brake Channel A pin
  }

//This function is called at the end of Setup to demo that our motors work.
void Demo(){
  //ServoLid works
  servoLid.write(130);
  delay(300);
  servoLid.write(170);
  delay(300);
  servoLid.write(130);
  delay(300);
  servoLid.write(170);
  delay(300);
  servoLid.write(130);
  delay(300);
  
  //ServoArm works
  servoArm.write(130);
  delay(300);
  servoArm.write(180);
  delay(300);
  servoArm.write(130);
  delay(300);
  servoArm.write(180);
  delay(300);
  servoArm.write(130);
  delay(1000);

  
  //Steppers work
  stepper.moveTo (true, zeroPosition);

  delay(1000);
  
  stepper.moveTo (false, zeroPosition - 1200);

  delay(1000);
  
  stepper.moveTo (true, zeroPosition - 600);

  delay(1000);

  servoArm.write(180);
  delay(300);
  servoLid.write(172);
  delay(300);
  
  
  //Wheels work
  //*Seite* Rad bewegt sich vorwärts
  digitalWrite(12, HIGH); //Establishes forward direction of Channel A
  digitalWrite(9, LOW);   //Disengage the Brake for Channel A
  analogWrite(3, 255);   //Spins the motor on Channel A at full speed

  //*Seite* Rad bewegt sich vorwärts
  digitalWrite(13, LOW);  //Establishes backward direction of Channel B
  digitalWrite(8, LOW);   //Disengage the Brake for Channel B
  analogWrite(11, 255);    //Spins the motor on Channel B at half speed

  delay(2000);

  digitalWrite(9, HIGH);  //Engage the Brake for Channel A
  digitalWrite(8, HIGH);  //Engage the Brake for Channel B

  delay(1000);
  
  //*Seite* Rad bewegt sich rückwärts
  digitalWrite(12, LOW); //Establishes forward direction of Channel A
  digitalWrite(9, LOW);   //Disengage the Brake for Channel A
  analogWrite(3, 255);   //Spins the motor on Channel A at full speed

  //*Seite* Rad bewegt sich rückwärts
  digitalWrite(13, HIGH);  //Establishes backward direction of Channel B
  digitalWrite(8, LOW);   //Disengage the Brake for Channel B
  analogWrite(11, 255);    //Spins the motor on Channel B at half speed

  delay(2000);

  digitalWrite(9, HIGH);  //Engage the Brake for Channel A
  digitalWrite(8, HIGH);  //Engage the Brake for Channel B
  }

void setup() {
  
  //Start Terminal for Debugging
  Serial.begin(9600);
  Serial.println("Setup..");
  
  //Buzzer Setup
  pinMode (buzzerPin, OUTPUT);
  
  //Define the Switch Pins
  defineSwitchPins();
  
  //Set up Servos
  servoSetup();

  //Set up Stepper
  stepperSetup();
  Serial.println("Stepper Done..");

  //Set up Wheels
  wheelSetup();

  Demo();

  NewLevel();
}

void loop() {
  Serial.println("Looping..");
  int buttonPressed = checkIfButtonPressed();
  if(buttonPressed){
    moveArmToSwitch(buttonPressed);
    //Damage();
  }
}
