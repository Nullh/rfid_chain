// Push a button to make a stepper
// Turn 90 degrees, then back

#include <Stepper.h>

// Our stepper motor driver pins
#define STP_PIN1      A5
#define STP_PIN2      A4
#define STP_PIN3      A3
#define STP_PIN4      A2

#define BTN_PIN       A0

const int stepsPerRevolution = 2050;  // change this to fit the number of steps per 
// revolution for your motor
//const int buttonPin = BTN_PIN; // trigger the action with this pin
const int ledPin = 13;

int buttonState = 0;

// initialize the stepper library on pins 8 through 11:
// coil2, coil4, coil3, coil1
//Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);
Stepper myStepper(stepsPerRevolution, STP_PIN1, STP_PIN2, STP_PIN3, STP_PIN4);

void setup() {
  // set the speed at 20 rpm:
  myStepper.setSpeed(20);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(BTN_PIN, INPUT);
}

void loop() {

  // read the state of the pushbutton value:
  buttonState = digitalRead(BTN_PIN);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
    turnThenBack(myStepper, 0.25, stepsPerRevolution, 0, 500);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
}

//turn the motor by the desired amount, pause, then turn back
// myStepper is your stepper object
// turnFraction is the amount to turn, as a fraction. E.g. 0.25 = quarter turn
// stepsPerRevolution is the number of steps for a complete revolution of your stepper
// clockwise is a bool. 1 = clockwise
// pauseTime is how long to wait before turning back
void turnThenBack(Stepper myStepper, float turnFraction, int stepsPerRevolution, bool clockwise, int pauseTime) {
  if (clockwise == 1) {
    // turn clockwise
    myStepper.step(stepsPerRevolution*turnFraction);
    delay(pauseTime);
    myStepper.step(-stepsPerRevolution*turnFraction);
    sleepMotor();
  } else {
    // turn anticlockwise
    myStepper.step(-stepsPerRevolution*turnFraction);
    delay(pauseTime);
    myStepper.step(stepsPerRevolution*turnFraction);
    sleepMotor();
  }
}

// Truns off the magnets in the stepper to
// prevent it getting hot
// THIS WILL ALLOW THE STEPPER to be turned manually!
void sleepMotor() {
  digitalWrite(STP_PIN1,LOW);
  digitalWrite(STP_PIN2,LOW);
  digitalWrite(STP_PIN3,LOW);
  digitalWrite(STP_PIN4,LOW);  
}

