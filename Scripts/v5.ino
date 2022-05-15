// Code for the UslessBox

// Include Libraries
#include <CheapStepper.h>  //Stepper Library
#include <Servo.h>         //Servo Library
#include <string.h>
#include <util/atomic.h>
#include "grove_two_rgb_led_matrix.h"
#include "pRNG.h"
#include "pitches.h"
#include "rtttl.h"
#include "songs.h"

#define DISPLAY_COLOR_RED 0x00 //33
#define DISPLAY_COLOR_GREEN 0x33

//----------------------
bool testmode = false;
bool drivemode = true;
bool instantWin = false;
bool mitKlammern = false;

bool beiStartlevel = false;
int startLevel = 4;
//----------------------

// Global Variables

Servo servoLid;                      // Define Lid Servo
Servo servoArm;                      // Define Arm Servo
GroveTwoRGBLedMatrixClass matrix;    // Define Matrix
CheapStepper stepper(0, 1, A2, A1);  // Define Stepper on pins A5, A4, A2, A1;
ProgmemPlayer player(A0);
pRNG prng;  // Randomness

int buzzerPin = A0;
int buttonState1, buttonState2, buttonState3, buttonState4, buttonState5;  // Initliazing Button States
int zeroPosition;  // Initializing zeroPosition

unsigned long previousMillisBlink = 0;

bool driveDirection = false;
bool mistake = false;

bool buttonOne = false;
bool buttonTwo = false;
bool buttonThree = false;
bool buttonFour = false;
bool buttonFive = false;

bool buttonOneOnce = false;
bool buttonTwoOnce = false;
bool buttonThreeOnce = false;
bool buttonFourOnce = false;
bool buttonFiveOnce = false;

// Box Variables
int health = 32;
float playerStrength = 16;
bool showingHealth = false;

float driveSpeed = 0;
float angryness = 0;

int level;

// Player Variables



// LEVEL BILDSCHIRME

uint64_t krone[] = {
  0xffffffffffffffff,
  0x1bffff1b1bffff1b,
  0x1b1bff1b1bff1b1b,
  0x1b1b1b1b1b1b1b1b,
  0x1b1b1b1b1b1b1b1b,
  0x1b1b1b1b1b1b1b1b,
  0xffffffffffffffff,
  0xffffffffffffffff
};

// LEVEL BILDSCHIRME ENDE

// SPEAKER START

// SONGS
void Win() {
  tone(buzzerPin, NOTE_C6, 125);
  delay(130);
  tone(buzzerPin, NOTE_D6, 125);
  delay(130);
  tone(buzzerPin, NOTE_E6, 125);
  delay(130);
  tone(buzzerPin, NOTE_F6, 125);
  delay(130);
  tone(buzzerPin, NOTE_F6, 125);
  delay(130);
  tone(buzzerPin, NOTE_G6, 125);
  delay(125);
  noTone(buzzerPin);
}

void DamageSound() {
  tone(buzzerPin, NOTE_F5);
  delay(100);
  tone(buzzerPin, NOTE_E5);
  delay(100);
  tone(buzzerPin, NOTE_D5);
  delay(100);
  tone(buzzerPin, NOTE_D5);
  delay(100);
  noTone(buzzerPin);
}

void WaitForMatrix() { delay(1000); }

void StartUp() {
  matrix.setDisplayOrientation(2);
  uint64_t logo[] = {

      0xfffffffffffffefe, 0xfffffffffffffefe, 0xfffffffffffffefe,
      0xffffffffffffadfe, 0xffffffffffffabfe, 0xffffffffffffabfe,
      0xfffffffffffffefe, 0xfffffffffffffefe,

      0xfffffffffefefefe, 0xfffffffffefefefe, 0xfffffffffefefefe,
      0xffffffffaeaeadfe, 0xfffffffffefeabfe, 0xffffffffabababfe,
      0xfffffffffefefefe, 0xfffffffffefefefe,

      0xfffffefefefefefe, 0xffffababfefefefe, 0xfffffefefefefefe,
      0xffffaeaeaeaeadfe, 0xfffffefefefeabfe, 0xfffffefeabababfe,
      0xffff16fefefefefe, 0xfffffefefefefefe,

      0xfefefefefefefefe, 0xfeabababfefefefe, 0xfeabfefefefefefe,
      0xfeaeaeaeaeaeadfe, 0xfefefefefefeabfe, 0xfe16fefeabababfe,
      0x161616fefefefefe, 0xfe16fefefefefefe};

  matrix.displayFrames(logo, 400, false, 1);
  tone(buzzerPin, NOTE_C5);
  delay(200);
  noTone(buzzerPin);
  delay(200);
  matrix.displayFrames(logo + 8, 400, false, 1);
  tone(buzzerPin, NOTE_F5);
  delay(200);
  noTone(buzzerPin);
  delay(200);
  matrix.displayFrames(logo + 16, 500, false, 1);
  tone(buzzerPin, NOTE_A5);
  delay(200);
  noTone(buzzerPin);
  delay(200);
  matrix.displayFrames(logo + 24, 500, true, 1);
  tone(buzzerPin, NOTE_F5);
  delay(800);
  noTone(buzzerPin);
}

void HealthFill(int i) {
  unsigned int healthTone = 440;
  for (int j = 0; j < i; j++) {
    healthTone *= 1.059463;  // goes up in half tones
  }
  tone(buzzerPin, healthTone, 300);
}

// Speaker END

// HELPING FUNCTIONS

int getRandomBinaryNumber(){
  return (prng.getRndInt()%2);
}


// END HELPING FUNCTIONS

// SETUP
void MatrixSetup() { matrix.displayClockwise(true, true, 9500, false); }

// PRE: Buttonstates initialised
// POST: Pinmodes
void DefineSwitchPins() {
  // Switch Pins, from left to right
  // Switch Pin 1
  pinMode(2, INPUT_PULLUP);
  // Switch Pin 2
  pinMode(4, INPUT_PULLUP);
  // Switch Pin 3
  pinMode(5, INPUT_PULLUP);
  // Switch Pin 4
  pinMode(7, INPUT_PULLUP);
  // Switch Pin 5
  pinMode(10, INPUT_PULLUP);
}

// PRE: Servos Initialized
// POST: ServoLid attaches to pin 6, ServoArm attaches to pin A3, they both go
// into their starting position.
void ServorSetup() {

  servoLid.attach(6);   // Attaches the Servo from the Lid to Pin 6

  servoArm.attach(A3);   // Attaches the Servo from the Arm to Pin A3

  servoArm.write(180);  // Hide Arm

  servoLid.write(172);  // Close Lid
}

// PRE: Assumes ~7V current, stepper is initialized
// POST: Stepper is ready to hit middle switch, zeroPosition is equal to fully
// to touching the right wall.
void StepperSetup() {

  stepper.setTotalSteps(4076);  // Motor is geared 63.68395:1 (measured) rather
                                // than 64:1 (advertised)
  StepperCalibration();

  // Move to Middle
  stepper.moveCCW(700);
}

// PRE: Stepper initialised
// POST: zeroPosition is equal to fully to touching the right wall.
void StepperCalibration() {

  // Calibration
  stepper.setRpm(18);  // 18 is strong enough to move the contraption, but as
                       // soon as it hits the wall stepper doesn't move anymore
  stepper.moveCW(1500);  // Stepper hits the right wall
  delay(100);

  zeroPosition = stepper.getStep();  // We remember where the stepper is right now
  delay(100);
}

void WheelSetup() {
  // Setup Channel A
  pinMode(12, OUTPUT);  // Initiates Motor Channel A pin
  pinMode(9, OUTPUT);   // Initiates Brake Channel A pin

  // Setup Channel B
  pinMode(13, OUTPUT);  // Initiates Motor Channel A pin
  pinMode(8, OUTPUT);   // Initiates Brake Channel A pin
}

// This function is called at the end of Setup to greet the user.
void Greeting() {
  bool finalStep = false;
  unsigned int currentStep = 0;
  unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  int songIndex = random(55);
  player.setSong(song_8);

  while (!finalStep) {
    int stepsLeft = stepper.getStepsLeft();
    currentMillis = millis();
    if (currentStep < 17) {
      player.pollSong();
    } else if (currentStep == 17) {
      player.silence();
    };

    stepper.run();
    if (currentStep < 12) {
      // SERVOLID START
      if (currentMillis - previousMillis >= 300 && currentStep == 0) {
        servoLid.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 1) {
        servoLid.write(170);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 2) {
        servoLid.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 3) {
        servoLid.write(170);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 4) {
        servoLid.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      // SERVOLID END

      else if (currentMillis - previousMillis >= 1000 && currentStep == 5) {
        ++currentStep;
      }

      // SERVOARM START
      else if (currentMillis - previousMillis >= 300 && currentStep == 6) {
        servoArm.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 7) {
        servoArm.write(180);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 8) {
        servoArm.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 9) {
        servoArm.write(180);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 10) {
        servoArm.write(130);
        previousMillis = currentMillis;
        ++currentStep;

      }

      else if (currentMillis - previousMillis >= 1000 && currentStep == 11) {
        ++currentStep;
      }
      // SERVOARM END
    } else {
      // STEPPER START
      if (currentMillis - previousMillis >= 0 && currentStep == 12) {
        stepper.newMoveCW(650);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 13) {
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 1500 && currentStep == 14 &&
               stepsLeft == 0) {
        stepper.newMoveCCW(1250);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 15) {
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 1500 && currentStep == 16 &&
               stepsLeft == 0) {
        stepper.newMoveCW(650);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 17 &&
               stepsLeft == 0) {
        ++currentStep;
      }
      // STEPPER END

      // WINK START
      else if (currentMillis - previousMillis >= 400 && currentStep == 18) {
        matrix.setDisplayOrientation(DISPLAY_ROTATE_0);
        matrix.displayEmoji(0, 1000, false);
        tone(buzzerPin, NOTE_E5, 150);
        servoArm.write(150);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 19) {
        tone(buzzerPin, NOTE_GS5, 150);
        servoArm.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 20) {
        tone(buzzerPin, NOTE_E5, 150);
        servoArm.write(150);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 21) {
        tone(buzzerPin, NOTE_GS5, 150);
        servoArm.write(130);
        previousMillis = currentMillis;
        ++currentStep;
      }

      // WINK END
      // HIDE ARM AND CLOSE LID START
      else if (currentMillis - previousMillis >= 1000 && currentStep == 22) {
        servoArm.write(180);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 300 && currentStep == 23) {
        servoLid.write(172);
        previousMillis = currentMillis;
        ++currentStep;
      }
      // HIDE ARM AND CLOSE LID END

      // FINALSTEP START
      else if (currentMillis - previousMillis >= 1000 && currentStep == 24) {
        ++currentStep;
        finalStep = true;
      }
      // FINALSTEP END
    }
  }
}

void setup() {
  Wire.begin();     // Matrix Wire Begins
  WaitForMatrix
();  // Waits one second
  // CombinationHandler();
  if (!testmode) StartUp();  // StartUp Sequence
  // Buzzer Setup
  randomSeed(prng.getRndLong());


  pinMode(buzzerPin, OUTPUT);
 
  // Define the Switch Pins
  DefineSwitchPins();

  // Set up Servos
  ServorSetup();

  // Set up Stepper
  StepperSetup();

  // Set up Wheels
  WheelSetup();

  MatrixSetup();
  if (!testmode) Greeting();
  if (instantWin){
    level = 10;
  } 
  else
  {
    level = 0;
  }
  if (beiStartlevel){
    level = (startLevel -1);
  }
  newLevel(++level);
}

// SETUP END

// BUTTONS

// PRE: buttonStates initialized
// POST: buttonStates are read and printed.
void ReadButtonState() {
  buttonState1 = digitalRead(2);
  buttonState2 = digitalRead(4);
  buttonState3 = digitalRead(5);
  buttonState4 = digitalRead(7);
  buttonState5 = digitalRead(10);
}

void MoveArmToSwitch(int buttonNumber) {
  // These two Variables are just for simplicity's sake. true -> stepper turns
  // clockwise -> arm moves to the right.
  bool moveToRight = true;
  bool moveToLeft = false;
  bool moveDir = moveToLeft;  // Last move in calibration is moving left

  // If Arm moves to wrong position, you can change the position here! If Arm is
  // left of button, decrease the position value. If Arm is right of button,
  // increase the position value. Attention! If the arm comes from the right,
  // steps are added to the position automatically, you can change it in the if
  // statements below of the respective buttons.
  int button1Position = zeroPosition - 1150;
  int button2Position = zeroPosition - 850;
  int button3Position = zeroPosition - 580;
  int button4Position = zeroPosition - 300;
  int button5Position = zeroPosition;
  int buttonPosition;

  switch (buttonNumber) {
    case 1:
      buttonPosition = button1Position;
      break;
    case 2:
      buttonPosition = button2Position;
      break;
    case 3:
      buttonPosition = button3Position;
      break;
    case 4:
      buttonPosition = button4Position;
      break;
    case 5:
      buttonPosition = button5Position;
      break;
  }

  if (stepper.getStep() < buttonPosition) {
    // If Arm is to the left of Switch
    stepper.moveTo(moveToRight, buttonPosition);
    moveDir = moveToRight;
  } else {
    stepper.moveTo(moveToLeft, buttonPosition);
    if (moveDir != moveToRight) {
      stepper.moveCCW(150);
    }
    moveDir = moveToLeft;
  }
  // Allowed to be outside because function is only called when button is
  // actually pressed.
  PressButton();
}

int CheckIfButtonPressed() {

  ReadButtonState();

  int buttonPressed;
  if (buttonState1 == LOW) {
    buttonPressed = 1;

    if (buttonOneOnce == true || buttonOne == false) 
      mistake = true;
    else if (buttonOne == true){
      buttonOneOnce = true;
      buttonOne = false;
      DamageSound();
    }

  } else if (buttonState2 == LOW) {
    buttonPressed = 2;

    if (buttonTwoOnce == true || buttonTwo == false) 
      mistake = true;
    else if (buttonTwo == true){
      buttonTwoOnce = true;
      buttonTwo = false;
      DamageSound();
    }

  } else if (buttonState3 == LOW) {
    buttonPressed = 3;

    if (buttonThreeOnce == true || buttonThree == false) 
      mistake = true;
    else if (buttonThree == true){
      buttonThreeOnce = true;
      buttonThree = false;
      DamageSound();
    }

  } else if (buttonState4 == LOW) {
    buttonPressed = 4;

    if (buttonFourOnce == true || buttonFour == false) 
      mistake = true;
    else if (buttonFour == true){
      buttonFourOnce = true;
      buttonFour = false;
      DamageSound();
    }

  } else if (buttonState5 == LOW) {
    buttonPressed = 5;

    if (buttonFiveOnce == true || buttonFive == false) 
      mistake = true;
    else if (buttonFive == true){
      buttonFiveOnce = true;
      buttonFive = false;
      DamageSound();
    }

  } else {
    buttonPressed = 0;
  }

  return buttonPressed;
}

void ButtonPressingCheck() {
  int buttonPressed = CheckIfButtonPressed();
  if (buttonPressed) {
    MoveArmToSwitch(buttonPressed);
  }
}

void buttonReset(){
  buttonOne = false;
  buttonTwo = false;
  buttonThree = false;
  buttonFour = false;
  buttonFive = false;

  buttonOneOnce = false;
  buttonTwoOnce = false;
  buttonThreeOnce = false;
  buttonFourOnce = false;
  buttonFiveOnce = false;
}


void OpenLid() {
  servoLid.write(130);
  delay(50);
}

void CloseLid() {
  servoLid.write(172);
  delay(50);
}

void OpenArm() {
  servoArm.write(110);
  delay(200);
}

void CloseArm() {
  servoArm.write(180);
  delay(50);
}

void PressButton() {
  // Open Lid
  OpenLid();

  // Press button
  OpenArm();

  // Hide Arm
  CloseArm();

  // Close Lid
  CloseLid();

  if (mistake){
    MistakeHandler();
    mistake = false;
  }

  if (buttonOneOnce && buttonTwoOnce && buttonThreeOnce && buttonFourOnce && buttonFiveOnce){
    DamageHandler();
  }

}

void  checkRandomNumber(int numberStelle, int generierteNummer){
  if (generierteNummer == 0){
    switch(numberStelle){
      case 1:
        buttonOne = false;
        buttonOneOnce = true;
        break;
      case 2:
        buttonTwo = false;
        buttonTwoOnce = true;
        break;
      case 3:
        buttonThree = false;
        buttonThreeOnce = true;
        break;
      case 4:
        buttonFour = false;
        buttonFourOnce = true;
        break;
      case 5:
        buttonFive = false;
        buttonFiveOnce = true;
        break;
    }
  } else if (generierteNummer == 1){
    switch(numberStelle){
      case 1:
        buttonOne = true;
        break;
      case 2:
        buttonTwo = true;
        break;
      case 3:
        buttonThree = true;
        break;
      case 4:
        buttonFour = true;
        break;
      case 5:
        buttonFive = true;
        break;
    }
  }

}

void BeginnShowBinary(int delaySeconds){
    matrix.displayString("[", 10000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
}

void EndShowBinary(int delaySeconds){
    matrix.displayString("]", 10000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
}

void GenerateNumbers(){

    int number = getRandomBinaryNumber();
    int delaySeconds = 1100;
    int delayMini = 100;

    if (mitKlammern) {BeginnShowBinary(delaySeconds);}
    
    number = getRandomBinaryNumber();
    checkRandomNumber(1,number);
    matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
    
    number = getRandomBinaryNumber();
    checkRandomNumber(2,number);
    matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
    delay(delayMini);
    matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
    
    number = getRandomBinaryNumber();
    checkRandomNumber(3,number);
    matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
    delay(delayMini);
    matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
    
    number = getRandomBinaryNumber();
    checkRandomNumber(4,number);
    matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
    delay(delayMini);
    matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
    
    // number = getRandomBinaryNumber();
    number = 1; // da sonst zu 00000 kommen kann
    checkRandomNumber(5,number);
    matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
    delay(delayMini);
    matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
    delay(delaySeconds);
    
    if (mitKlammern) {EndShowBinary(delaySeconds);}

}

void ShowNumbers(){
  int delaySeconds = 1100;
  int delayMini = 100;
  int number;

  matrix.setDisplayOrientation(0);
  if (mitKlammern){BeginnShowBinary(delaySeconds);}
  

  number = buttonOne;
  matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
  delay(delaySeconds);

  number = buttonTwo;
  matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
  delay(delayMini);
  matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
  delay(delaySeconds);

  number = buttonThree;
  matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
  delay(delayMini);
  matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
  delay(delaySeconds);

  number = buttonFour;
  matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
  delay(delayMini);
  matrix.displayNumber(number, 3000, true, DISPLAY_COLOR_RED);
  delay(delaySeconds);

  number = buttonFour;
  matrix.displayString("|", 10000, true, DISPLAY_COLOR_RED);
  delay(delayMini);
  matrix.displayNumber(buttonFive, 3000, true, DISPLAY_COLOR_RED);
  delay(delaySeconds);

  if (mitKlammern) {EndShowBinary(delaySeconds);}
}


// BUTTONS END


// Driving

void Driving() {
  // Motor A forward @ currentDriveSpeed
  digitalWrite(12, driveDirection);    // Establishes direction of Channel A
  digitalWrite(9, LOW);                // Disengage the Brake for Channel A
  analogWrite(3, driveSpeed * 255);          // Spins the motor on Channel A 

  // Motor B forward @ currentDriveSpeed
  digitalWrite(13, !driveDirection);  // Establishes direction of Channel B
  digitalWrite(8, LOW);               // Disengage the Brake for Channel B
  analogWrite(11, driveSpeed * 255);        // Spins the motor on Channel B
  delay(200);
}

void CheckIfDirectionChange() {
  int changeDirection = random(10000);

  if (changeDirection == 0) {
    driveDirection = !driveDirection;
    digitalWrite(9, HIGH);  // Engage the Brake for Channel A
    digitalWrite(8, HIGH);
    if (drivemode) {
       Driving();
    }
       
  }
}

// Driving END

// GAMESYSTEM

void HealthFill() {
  matrix.setDisplayOrientation(0);
  matrix.displayEmoji(11, 1000, false);
  delay(250);
  matrix.displayEmoji(10, 1000, true);
  delay(250);

  matrix.setDisplayOrientation(1);
  health = 32;
  for (int i = 0; i <= 16; ++i) {
    HealthFill(i);
    matrix.displayBar(i * 2, 60, true, 0x00);
    delay(60);
  }
}


void matrixLevel(int levelCase){

  matrix.setDisplayOrientation(0);

  matrix.displayNumber(levelCase, 2000, true, DISPLAY_COLOR_GREEN);

  delay(2000);
}


void newLevel(int level) {

  buttonReset();

  HealthFill();
  showingHealth = false;
  switch (level) {
    case 1:
      matrixLevel(1);
      driveSpeed = 0;
      angryness = 0;
      playerStrength = 32;
      GenerateNumbers();
      break;
    case 2:
      matrixLevel(2);
      driveSpeed = 0;
      angryness = 0;
      playerStrength = 16;
      GenerateNumbers();
      break;
    case 3:
      matrixLevel(3);
      driveSpeed = 0;
      angryness = 0;
      playerStrength = 11;
      GenerateNumbers();
      break;
    case 4:
      matrixLevel(4);
      driveSpeed = 0.1;
      angryness = 0;
      playerStrength = 11;
      GenerateNumbers();
      break;
    case 5:
      matrixLevel(5);
      driveSpeed = 0.25;
      angryness = 10;
      playerStrength = 11;
      GenerateNumbers();
      break;
    case 6:
      matrixLevel(6);
      driveSpeed = 0.4;
      angryness = 20;
      playerStrength = 10;
      GenerateNumbers();
      break;
    case 7:
      matrixLevel(7);
      driveSpeed = 0.5;
      angryness = 30;
      playerStrength = 8;
      GenerateNumbers();
      break;
    case 8:
      matrixLevel(8);
      driveSpeed = 0.6;
      angryness = 40;
      playerStrength = 5;
      GenerateNumbers();
      break;
    case 9:
      matrixLevel(9);
      driveSpeed = 0.7;
      angryness = 60;
      playerStrength = 4;
      GenerateNumbers();
      break;
    case 10:
      matrixLevel(10);
      driveSpeed = 1;
      angryness = 80;
      playerStrength = 2;
      GenerateNumbers();
      break;
    case 11:
      WinLevel();
      break;
  }
}

void CheckCurrentEmotion(int& currentEmotion) {
  if (level <= 2) {
    currentEmotion = 0;
  } else if (angryness <= 10) {
    currentEmotion = 7;
  } else if (angryness <= 30) {
    currentEmotion = 8;
  } else if (angryness <= 60) {
    LightAngry();
    currentEmotion = 3;
  } else if (angryness <= 90) {
    Angry();
    currentEmotion = 4;
  } else if (angryness <= 100) {
    VeryAngry();
    currentEmotion = 25;
  }
}

void LightAngry(){
      OpenLid();
      CloseLid();
}
void Angry(){
  LightAngry();
  LightAngry();
}

void VeryAngry(){
  LightAngry();
  OpenLid();
  delay(200);
  CloseLid();
  LightAngry();
}

void ShowHealth() {
  matrix.setDisplayOrientation(1);
  matrix.displayBar(health, 60, true, 0x00);
}

void ShowEnemy() {
  int currentEmotion;
  matrix.setDisplayOrientation(0);
  CheckCurrentEmotion(currentEmotion);
  matrix.displayEmoji(currentEmotion, 60, true);
}

void HealthEnemyChanging(unsigned long& currentMillisBlink) {
  if (currentMillisBlink - previousMillisBlink >= 2300) {
    previousMillisBlink = currentMillisBlink;
    if (showingHealth == true) {
      ShowEnemy();
      showingHealth = false;
    } else {
      ShowHealth();
      showingHealth = true;
    }
  }
}

void TakesDamage(){
    matrix.displayEmoji(12, 600, true);
    DamageSound();
    delay(600);
}


// PRE: alive
// POST: dead
void Death() {
 
  matrix.displayEmoji(5, 600, true);
  DamageSound();
  delay(1000);
  
  newLevel(++level);
}

// PRE: Player fulfilled task
// POST: Box received damage
void DamageHandler() {
  matrix.setDisplayOrientation(0);
  health -= playerStrength;
  angryness += 15;
  if (health > 0) {
      TakesDamage();
      showingHealth = false;
      buttonReset();
      GenerateNumbers();

  } else {
      Death();
  }
}

void MistakeHandler(){
  if (health != 32){
    health += playerStrength;
  }
  ShowHealth();
  delay(1000);
  ShowNumbers();

  
}

void WinLevel(){
  Win();
  matrix.setDisplayOrientation(0);
  matrix.displayFrames(krone, 2000, true, 1);

  player.setSong(song_145);
  
  while (true){
    player.pollSong();
  }
}

// END GAMESYSTEM

// LOOP
void TickCheck(unsigned long& currentMillisBlink) {
  ButtonPressingCheck();

  HealthEnemyChanging(currentMillisBlink);

  CheckIfDirectionChange();
}

void loop() {
  unsigned long currentMillisBlink = millis();

  TickCheck(currentMillisBlink);
}
// LOOP END
