//A Demo for Tuesday.

//Include Libraries
#include <Servo.h> //Servo Library
#include <CheapStepper.h> //Stepper Library
#include "pitches.h"
#include "grove_two_rgb_led_matrix.h"
#include <rtttl.h>
#include "songs.h"
#include <util/atomic.h>
#include "pRNG.h"
#include <string.h>

//Global Variables
Servo servoLid;  // Define Lid Servo
Servo servoArm;  // Define Arm Servo
CheapStepper stepper (0, 1, A2, A1); // Define Stepper on pins A5, A4, A2, A1;
ProgmemPlayer player(A0);
pRNG prng; //Randomness

int buzzerPin = A0;
int buttonState1, buttonState2, buttonState3, buttonState4, buttonState5; // Initliazing Button States
int zeroPosition; //Initializing zeroPosition
bool driveDirection = false;
unsigned long previousMillisBlink = 0;

int level = 0;

//Box Variables
int health = 32;
float playerStrength = 16;
bool showingHealth = false;

float driveSpeed = 0;
float angryness = 0;

//Player Variables

GroveTwoRGBLedMatrixClass matrix;

//SPEAKER START

//SONGS
void Win () {
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

void DamageSound () {
  tone(buzzerPin, NOTE_F5);
  delay(100);
  tone(buzzerPin, NOTE_E5);
  delay(100);
  tone(buzzerPin, NOTE_D5);
  delay(100);
  noTone(buzzerPin);
}

void waitForMatrix() {
  delay(1000);
}

void startUp() {
  matrix.setDisplayOrientation(2);
  uint64_t logo[] = {

    0xfffffffffffffefe,
    0xfffffffffffffefe,
    0xfffffffffffffefe,
    0xffffffffffffadfe,
    0xffffffffffffabfe,
    0xffffffffffffabfe,
    0xfffffffffffffefe,
    0xfffffffffffffefe,

    0xfffffffffefefefe,
    0xfffffffffefefefe,
    0xfffffffffefefefe,
    0xffffffffaeaeadfe,
    0xfffffffffefeabfe,
    0xffffffffabababfe,
    0xfffffffffefefefe,
    0xfffffffffefefefe,

    0xfffffefefefefefe,
    0xffffababfefefefe,
    0xfffffefefefefefe,
    0xffffaeaeaeaeadfe,
    0xfffffefefefeabfe,
    0xfffffefeabababfe,
    0xffff16fefefefefe,
    0xfffffefefefefefe,

    0xfefefefefefefefe,
    0xfeabababfefefefe,
    0xfeabfefefefefefe,
    0xfeaeaeaeaeaeadfe,
    0xfefefefefefeabfe,
    0xfe16fefeabababfe,
    0x161616fefefefefe,
    0xfe16fefefefefefe
  };
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

void healthFill(int i) {
  unsigned int healthTone = 440;
  for (int j = 0; j < i; j++) {
    healthTone *= 1.059463; //goes up in half tones
  }
  tone(buzzerPin, healthTone, 300);
}
//Speaker END

//PRE: Buttonstates initialised
//POST: Pinmodes
void defineSwitchPins() {
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
void servoSetup() {
  //Serial.println("Servo Setup..");
  //Attaches the Servo from the Lid to Pin 6
  servoLid.attach(6);
  //Attaches the Servo from the Arm to Pin A3
  servoArm.attach(A3);

  servoArm.write(180); //Hide Arm

  servoLid.write(172); //Close Lid
}

//PRE: Assumes ~7V current, stepper is initialized
//POST: Stepper is ready to hit middle switch, zeroPosition is equal to fully to touching the right wall.
void stepperSetup() {
  //Serial.println("Stepper Setup..");
  stepper.setTotalSteps(4076); // Motor is geared 63.68395:1 (measured) rather than 64:1 (advertised)
  stepperCalibration();

  //Move to Middle
  stepper.moveCCW (700);
}

//PRE: Stepper initialised
//POST: zeroPosition is equal to fully to touching the right wall.
void stepperCalibration() {
  //Serial.println("Stepper Calibration..");
  //Calibration
  stepper.setRpm(18); //18 is strong enough to move the contraption, but as soon as it hits the wall stepper doesn't move anymore
  stepper.moveCW(1500); //Stepper hits the right wall
  delay(100);
  zeroPosition = stepper.getStep(); //We remember where the stepper is right now
  delay(100);
}

//PRE: buttonStates initialized
//POST: buttonStates are read and printed.
void readButtonStates() {
  buttonState1 = digitalRead(2);
  buttonState2 = digitalRead(4);
  buttonState3 = digitalRead(5);
  buttonState4 = digitalRead(7);
  buttonState5 = digitalRead(10);
}

void moveArmToSwitch(int buttonNumber) {
  //These two Variables are just for simplicity's sake. true -> stepper turns clockwise -> arm moves to the right.
  bool moveToRight = true;
  bool moveToLeft = false;
  bool moveDir = moveToLeft; //Last move in calibration is moving left

  //If Arm moves to wrong position, you can change the position here! If Arm is left of button, decrease the position value. If Arm is right of button, increase the position value.
  //Attention! If the arm comes from the right, steps are added to the position automatically, you can change it in the if statements below of the respective buttons.
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
    //If Arm is to the left of Switch
    stepper.moveTo (moveToRight, buttonPosition);
    moveDir = moveToRight;
  }
  else {
    stepper.moveTo (moveToLeft, buttonPosition);
    if (moveDir != moveToRight) {
      stepper.moveCCW(150);
    }
    moveDir = moveToLeft;
  }
  //Allowed to be outside because function is only called when button is actually pressed.
  pressButton();
}

int checkIfButtonPressed() {
  readButtonStates();
  int buttonPressed;
  if (buttonState1 == LOW) {
    buttonPressed = 1;

  } else if (buttonState2 == LOW) {
    buttonPressed = 2;
  } else if (buttonState3 == LOW) {
    buttonPressed = 3;
  } else if (buttonState4 == LOW) {
    buttonPressed = 4;
  } else if (buttonState5 == LOW) {
    buttonPressed = 5;
  } else {
    buttonPressed = 0;
  }
  return buttonPressed;
}

void openLid() {
  servoLid.write(130);
  delay(50);
}

void closeLid() {
  servoLid.write(172);
  delay(50);
}

void openArm() {
  servoArm.write(110);
  delay(200);
}

void closeArm() {
  servoArm.write(180);
  delay(50);
}

void pressButton() {
  //Open Lid
  openLid();

  //Press button
  openArm();

  //Hide Arm
  closeArm();

  tickCheck();
  
  //Close Lid
  closeLid();
}

void wheelSetup() {
  //Setup Channel A
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT); //Initiates Brake Channel A pin

  //Setup Channel B
  pinMode(13, OUTPUT); //Initiates Motor Channel A pin
  pinMode(8, OUTPUT);  //Initiates Brake Channel A pin
}

//This function is called at the end of Setup to greet the user.
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
    if(currentStep < 17) {
      player.pollSong();
      }
    else if(currentStep == 17){
      player.silence();
      };
    
    stepper.run();
    if (currentStep < 12) {
      //SERVOLID START
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

      //SERVOLID END

      else if (currentMillis - previousMillis >= 1000 && currentStep == 5) {
        ++currentStep;
      }

      //SERVOARM START
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
      //SERVOARM END
    } else {
      //STEPPER START
      if (currentMillis - previousMillis >= 0 && currentStep == 12) {
        stepper.newMoveCW(650);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 13) {
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 1500 && currentStep == 14 && stepsLeft == 0) {
        stepper.newMoveCCW(1250);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 15) {
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 1500 && currentStep == 16 && stepsLeft == 0) {
        stepper.newMoveCW(650);
        previousMillis = currentMillis;
        ++currentStep;
      }

      else if (currentMillis - previousMillis >= 200 && currentStep == 17 && stepsLeft == 0) {
        ++currentStep;
      }
      //STEPPER END

      //WINK START
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

      //WINK END
      //HIDE ARM AND CLOSE LID START
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
      //HIDE ARM AND CLOSE LID END

      //FINALSTEP START
      else if (currentMillis - previousMillis >= 1000 && currentStep == 24) {
        ++currentStep;
        finalStep = true;

      }
      //FINALSTEP END
    }
  }

}
long long convert(int n) {
  long long bin = 0;
  int rem, i = 1;

  while (n!=0) {
    rem = n % 2;
    n /= 2;
    bin += rem * i;
    i *= 10;
  }

  return bin;
}

void combinationHandler(){
  int currentCombination = convert(prng.getRndInt()%256);
  matrix.displayNumber(currentCombination, 3000, true, 0xfe);
  delay(10000);
  }
void healthFill(){
  matrix.setDisplayOrientation(0);
  matrix.displayEmoji(11, 1000, false);
  delay(250);
  matrix.displayEmoji(10, 1000, true);
  delay(250);

  matrix.setDisplayOrientation(1);
  health = 32;
  for (int i = 0; i <= 16; ++i) {
    healthFill(i);
    matrix.displayBar(i * 2, 60 , true, 0x00);
    delay(60);
  }
}

void newLevel(int level){
  healthFill();
  showingHealth = false;
  switch(level){
    case 1:
      driveSpeed = 0;
      angryness = 0;
      playerStrength = 16;
      break;
    case 2:
      driveSpeed = 0;
      angryness = 0;
      playerStrength = 16;
      break;
    case 3:
      driveSpeed = 0;
      angryness = 0;
      playerStrength = 11;
      break;
    case 4:
      driveSpeed = 10;
      angryness = 0;
      playerStrength = 11;
      break;
    case 5:
      driveSpeed = 25;
      angryness = 10;
      playerStrength = 11;
      break;
    case 6:
      driveSpeed = 40;
      angryness = 20;
      playerStrength = 10;
      break;
    case 7:
      driveSpeed = 50;
      angryness = 30;
      playerStrength = 8;
      break;
    case 8:
      driveSpeed = 60;
      angryness = 40;
      playerStrength = 5;
      break;
    case 9:
      driveSpeed = 70;
      angryness = 60;
      playerStrength = 4;
      break;
    case 10:
      driveSpeed = 100;
      angryness = 80;
      playerStrength = 2;
      break;
    case 11:
      //winLevel();
      break;
  }
  
}

void checkCurrentEmotion(int& currentEmotion){
  if(level <= 2){
    currentEmotion = 0;
  }
  else if (angryness <= 10){
    currentEmotion = 7;
  }
  else if (angryness <= 30){
    currentEmotion = 8;
  }
  else if (angryness <= 60){
    currentEmotion = 3;
  }
  else if (angryness <= 90){
    currentEmotion = 4;
  }
  else if (angryness <= 100){
    currentEmotion = 25;
  }
}

void showHealth(){
  matrix.setDisplayOrientation(1);
  matrix.displayBar(health, 60, true, 0x00);
}

void showEnemy(){
  int currentEmotion;
  matrix.setDisplayOrientation(0);
  checkCurrentEmotion(currentEmotion);
  matrix.displayEmoji(currentEmotion, 60, true);
}

void healthEnemyBlink(unsigned long& currentMillisBlink){
  if (currentMillisBlink - previousMillisBlink >= 2000){
    previousMillisBlink = currentMillisBlink;
    if (showingHealth == true) {
      showEnemy();
      showingHealth = false;
    } else {
      showHealth();
      showingHealth = true;
    }
  }
}

void checkIfDirectionChange(){
  int changeDirection = random(10000);
    if(changeDirection == 0){
    driveDirection = !driveDirection;
    digitalWrite(9, HIGH);  //Engage the Brake for Channel A
    digitalWrite(8, HIGH);
    //driving();
  }
}
void driving(){
    //Motor A forward @ currentDriveSpeed
    digitalWrite(12, driveDirection);  //Establishes backward direction of Channel A
    digitalWrite(9, LOW);   //Disengage the Brake for Channel A
    analogWrite(3, driveSpeed);    //Spins the motor on Channel A at half speed
    
    //Motor B forward @ currentDriveSpeed
    digitalWrite(13, !driveDirection); //Establishes forward direction of Channel B
    digitalWrite(8, LOW);   //Disengage the Brake for Channel B
    analogWrite(11, driveSpeed);   //Spins the motor on Channel B at full speed
    delay(200);

  
}

//PRE: alive
//POST: dead
void death(){
  newLevel(++level);
  
  }
//PRE: Player fulfilled task
//POST: Box received damage
void damageHandler(){
  matrix.setDisplayOrientation(0);
  matrix.displayEmoji(12, 400, true);
  health -= playerStrength;
  delay(400);
  
  if(health > 0){
    showingHealth = false;
  } else{
    death();
  }
  
}

void tickCheck(unsigned long& currentMillisBlink){
  int buttonPressed = checkIfButtonPressed();
  if (buttonPressed) {
    moveArmToSwitch(buttonPressed);
  }
  healthEnemyBlink(currentMillisBlink);
  
  checkIfDirectionChange();
}

void tickCheck(){
  int buttonPressed = checkIfButtonPressed();
  if (buttonPressed) {
    moveArmToSwitch(buttonPressed);
  }
}

void setup() {
  Wire.begin(); // Matrix Wire Begins
  waitForMatrix();// Waits one second
  combinationHandler();
  //startUp(); // Startup Sequence
  //Buzzer Setup
  randomSeed(prng.getRndLong());
  
  pinMode (buzzerPin, OUTPUT);

  //Define the Switch Pins
  defineSwitchPins();

  //Set up Servos
  servoSetup();

  //Set up Stepper
  stepperSetup();

  //Set up Wheels
  wheelSetup();

  matrix.displayClockwise(true, true, 9500, false);

  //Greeting();

  newLevel(++level);
}

void loop() {
  unsigned long currentMillisBlink = millis();
  tickCheck(currentMillisBlink);
  
}
