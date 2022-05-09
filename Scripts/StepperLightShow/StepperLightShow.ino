#include <CheapStepper.h> //Stepper Library
#include <Servo.h> //Servo Library

CheapStepper stepper (A1,A2,A4,A5); // Define Stepper on pins A1, A2, A4, A5

Servo servoLid;  // Define Lid Servo
Servo servoArm;  // Define Arm Servo

int zeroPosition; //Initializing zeroPosition

  
void setup() {
  //Attaches the Servo from the Lid to Pin 6
    servoLid.attach(6);
    //Attaches the Servo from the Arm to Pin A3
    servoArm.attach(A3);

    servoArm.write(180); //Arm Closed

    servoLid.write(172);  // Lid Closed
  

}

void loop() {
  stepper.moveCCW (1);
  delay(30);
}
