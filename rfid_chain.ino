//////////////////////////////////////////////////////////////
// rfid_chain
//////////////////////////////////////////////////////////////
// NEEDS RENAMING!!
// Neil Holmes 2016, 2017
//////////////////////////////////////////////////////////////
// Checks for the presence of a valid RFID tag on each of
// 3 connected MFRC522 RFID readers. If the correct 
// tags are prsent, it turns a stepper motor to unlock
// a door.
//////////////////////////////////////////////////////////////

#include <SPI.h>
#include <MFRC522.h>
#include <Stepper.h>

// Pins 11, 12, 13 reserved for SPI
// These pins are not configurable
// 13 - SCK
// 12 - MISO
// 11 - MOSI
// connect the boards to these three pins, forming a bus
// https://www.arduino.cc/en/Reference/SPI

// These pins control the slave select and reset of each board
#define RST_PIN1         9
#define SS_PIN1          10
#define RST_PIN2         3
#define SS_PIN2          2 
#define RST_PIN3         6
#define SS_PIN3          5 

// Our stepper motor driver pins
// When wiring the stepper motor, check
// your data sheet for correct wiring
#define STP_PIN1         A5
#define STP_PIN2         A4
#define STP_PIN3         A3
#define STP_PIN4         A2

// Override Pins
#define OPEN_PIN         A0
// Wire up another switch to the door to 
// on this pin make it automatically lock 
// on close \/
#define CLOSE_PIN        A1 

// Set up the 3 RFID boards
MFRC522 board3(SS_PIN3, RST_PIN3); 
MFRC522 board2(SS_PIN2, RST_PIN2);  
MFRC522 board1(SS_PIN1, RST_PIN1);  

// Set up the vars to hold the valid card ids
String ValidRFID1;
String ValidRFID2;
String ValidRFID3;

// Set the initial state of the system -
// No RFIDs vaid and lock engaged
bool unlocked = 0;
bool lockState = 1; // We assume the lock is engaged when starting up

// Vars to hold if each board has a valid tag on it
bool board1Valid = 0;
bool board2Valid = 0;
bool board3Valid = 0;

// Init the state of the override buttons
int openOverride = 0;
int closeOverride = 0; // Connect the door closed sensor to the same pin

// Set up the stepper motor
const int stepsPerRevolution = 2050;
Stepper myStepper(stepsPerRevolution, STP_PIN1, STP_PIN2, STP_PIN3, STP_PIN4);


void setup() {

    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

    // Init SPI bus
    SPI.begin();        
    
    // Init MFRC522 cards
    board1.PCD_Init();
    board2.PCD_Init();
    board3.PCD_Init();

    // Set pin modes
    pinMode(SS_PIN1, OUTPUT);
    pinMode(SS_PIN2, OUTPUT);
    pinMode(SS_PIN3, OUTPUT);
    pinMode(OPEN_PIN, INPUT);
    pinMode(CLOSE_PIN, INPUT);

    // set the stepper speed at 20 rpm:
    myStepper.setSpeed(20);
    
    // Set tha valid card ids
    ValidRFID1 = "108792ab";
    ValidRFID2 = "5a9092ab";
    ValidRFID3 = "168a8bab";

    //Serial.print("Valid card id: ");
    //dump_byte_array(ValidRFID1, 4);
    //Serial.print(ValidRFID1);
    //Serial.println();

    // Add a delay just to let the SPI bus get in sync
    delay(10);
}

void loop() {

    // Check the state of the override buttons
    openOverride = digitalRead(OPEN_PIN);
    closeOverride = digitalRead(CLOSE_PIN);

    // If closeOverride is pressed, close the lock (if open)
    if (closeOverride == HIGH && lockState == 0) {
      turn(myStepper, -0.25, stepsPerRevolution);
      lockState = 1;
    }
    // If openOverride is pressed, open the lock (if closed)
    if (openOverride == HIGH && lockState == 1) {
      turn(myStepper, 0.25, stepsPerRevolution);
      //delay(1000);
      lockState = 0;
    }


    // Poll each board and check if there is a valid tag on it
    digitalWrite(SS_PIN1, LOW);
    board1Valid = check_card(board1);
    digitalWrite(SS_PIN1, HIGH);

    digitalWrite(SS_PIN2, LOW);
    board2Valid = check_card(board2);
    digitalWrite(SS_PIN2, HIGH);

    digitalWrite(SS_PIN3, LOW);
    board3Valid = check_card(board3);
    digitalWrite(SS_PIN3, HIGH);

    //Debug output of card readers
    Serial.println(String(board1Valid, BIN) + String(board2Valid, BIN) + String(board3Valid, BIN));

    //Check if we meet the unlock requirements
    // Sets unlock to 1 if we do
    unlocked = check_unlock();

    // Unlock if we have valid cars on all 3 readers
    // and the lock is not already open
    if (unlocked == 1 && lockState == 1) {
      turn(myStepper, 0.25, stepsPerRevolution);
      lockState = 0;
    }

    // Wait just to allow bandwidth
    delay(5);
}

// Wake up and read any cards on the board
bool check_card(MFRC522 board) {
    bool ret = 0;
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);
    
    // This sets any HALTed or IDLE cards to READY so they can be read
    board.PICC_WakeupA(bufferATQA, &bufferSize);
    
    // Grab the serial off the card 
    if ( ! board.PICC_ReadCardSerial())
        return 0;
        
    //Serial.print(F("Card UID:"));
    //Serial.println(card_id_to_string(board.uid.uidByte, board.uid.size));

    // Check if we have a valid card on the reader
    ret = is_card_valid(card_id_to_string(board.uid.uidByte, board.uid.size));

    // Puts the card back to HALT
    board.PICC_HaltA();
    
    return ret;
}

// Converts a card ID byte array to a string
String card_id_to_string(byte *card, byte bufferSize) {
  String id = "";
  for (byte i = 0; i < bufferSize; i++) {
      id = id + String(card[i], HEX);
      //Serial.println(id);
  }
  return id;
}

// Returns true if a card ID is accepted
bool is_card_valid(String cardId) {
  if (cardId == ValidRFID1 || cardId == ValidRFID2 || cardId == ValidRFID3) {
    //Serial.print("Card Valid\n");
    return 1;
  }
  else {
    return 0;
  }
}

// Returns true if all three boards have valid cards on them
bool check_unlock(){
  if (board1Valid == 1 && board2Valid == 1 && board3Valid == 1) {
    Serial.println("Unlocked!");
    return 1;
  }
  else {
    return 0;
  }
}

//turn the motor by the desired amount
// myStepper is your stepper object
// turnFraction is the amount to turn, as a fraction. E.g. 0.25 = quarter turn
// -0.5 = half turn backwards
// stepsPerRevolution is the number of steps for a complete revolution of your stepper
void turn(Stepper myStepper, float turnFraction, int stepsPerRevolution) {
    myStepper.step(stepsPerRevolution*turnFraction);
    sleepMotor();
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



