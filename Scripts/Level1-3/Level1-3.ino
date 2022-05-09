//Include Libraries
#include <Servo.h>
#include <time.h>       /* time */

//Start of Including MATRIX
#include "grove_two_rgb_led_matrix.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif


#define DISPLAY_COLOR    0X11
//End of Including MATRIX

//Define Classes
GroveTwoRGBLedMatrixClass matrix;

//Global Variables
Servo myservo;  // create servo object to control a servo
int buttonState = 0;
unsigned int level = 1;
float driveSpeed = 0;
float healthFactor = 0.1;
bool damageDone = false;

//Function Declarations
void servoArm();
void waitForMatrixReady();

void setup() {
  //Generate Random Seed
  
  //Switch Pin
  pinMode(A2, INPUT);

  //Switch Pin
  pinMode(A3, OUTPUT);

  //Servo Pin
  //myservo.attach(A3);

  //Matrix Begin
  Wire.begin();
    SERIAL.begin(115200);
    waitForMatrixReady();
    uint16_t VID = 0;
    VID = matrix.getDeviceVID();
    if (VID != 0x2886) {
        SERIAL.println("Can not detect led matrix!!!");
        while (1);
    }
   SERIAL.println("Matrix init success!!!");
   //Matrix End

  //Start Levels
  Serial.begin(9600);
}

void loop() {
  switch(level){
    case 1:
      driveSpeed = 0;
      healthFactor = 0.2;
      
      break;
    case 2:
      driveSpeed = 0.4;
      healthFactor = 0.4;
      
      break;
    case 3:
      driveSpeed = 1;
      healthFactor = 1;
      
      break;
    }
    runLevel();
}

void runLevel(){
  //Local Variables
  int currentDriveSpeed = driveSpeed * 255;
  int currentHealth = (healthFactor * 100);
  int maxHealth = currentHealth;
  bool driveDirection = false;
  
  SERIAL.println("healthFactor Defined as ");
  SERIAL.println(healthFactor);
  SERIAL.println("maxHealth Defined as ");
  SERIAL.println(maxHealth);
  SERIAL.println("CurrentHealth Defined as ");
  SERIAL.println(currentHealth);

  //Start of a level
  tone(A3, 784);
  delay(150);
  noTone(A3);
  delay(150);
  tone(A3, 784);
  delay(75);
  noTone(A3);
  delay(75);
  tone(A3, 784);
  delay(75);
  noTone(A3);
  delay(75);
  tone(A3, 1568);
  delay(500);
  noTone(A3);
  while(currentHealth > 0){
    
    //Check if Switch has been pressed
    buttonState = digitalRead(A2);
    if(buttonState == LOW && !damageDone){
      damageDone = true;
      tone(A3, 1760);
      delay(75);
      noTone(A3);
      delay(75);
      tone(A3, 1319);
      delay(75);
      noTone(A3);
      delay(75);

      currentHealth -= 10;
      matrix.displayEmoji(12, 4000, false);
      SERIAL.print("Damage done! CurrentHealth = ");
      SERIAL.println(currentHealth);
      servoArm();
    }

    //If currenhealth below 50% Machine gets Mad, else it's happy. Switch can't be pressed during that time
    if(currentHealth <= maxHealth / 2 && buttonState != LOW){
      matrix.displayEmoji(4, 5000, true);
    } else if(buttonState != LOW){
      matrix.displayEmoji(0, 5000, true);
    }
  
    //Try to drive away!
    int changeDirection = random(24);
    if(changeDirection == 0){
      driveDirection = !driveDirection;

      digitalWrite(9, HIGH);  //Engage the Brake for Channel A
      digitalWrite(8, HIGH);  //Engage the Brake for Channel B
      
      }
    //Motor A forward @ currentDriveSpeed
    digitalWrite(12, driveDirection);  //Establishes backward direction of Channel A
    digitalWrite(9, LOW);   //Disengage the Brake for Channel A
    analogWrite(3, currentDriveSpeed);    //Spins the motor on Channel A at half speed
    
    //Motor B forward @ currentDriveSpeed
    digitalWrite(13, !driveDirection); //Establishes forward direction of Channel B
    digitalWrite(8, LOW);   //Disengage the Brake for Channel B
    analogWrite(11, currentDriveSpeed);   //Spins the motor on Channel B at full speed
    delay(200);

    
    }
  ++level;
  matrix.displayEmoji(2, 2000, true);
  SERIAL.println("DEAD");
  delay(5000);
  }

void servoArm(){
  //Local Variables
  int pos = 0; 
  int schluss = 45;
  int anfang = 80;
  delay(2000);
  damageDone = false;
}

void waitForMatrixReady() {
    delay(1000);
}
 
