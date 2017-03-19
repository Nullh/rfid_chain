#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN1         9           // Configurable, see typical pin layout above
#define SS_PIN1          10          // Configurable, see typical pin layout above

#define RST_PIN2         3           // Configurable, see typical pin layout above
#define SS_PIN2          2 

#define RST_PIN3         6           // Configurable, see typical pin layout above
#define SS_PIN3          5 

#define UNLOCK_PIN       7


MFRC522 board3(SS_PIN3, RST_PIN3); 
MFRC522 board2(SS_PIN2, RST_PIN2);  
MFRC522 board1(SS_PIN1, RST_PIN1);  

String ValidRFID1;
String ValidRFID2;
String ValidRFID3;

bool unlocked = 0;

bool board1Valid = 0;
bool board2Valid = 0;
bool board3Valid = 0;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    board1.PCD_Init(); // Init MFRC522 card
    board2.PCD_Init();
    board3.PCD_Init();
    pinMode(SS_PIN1, OUTPUT);
    pinMode(SS_PIN2, OUTPUT);
    pinMode(SS_PIN3, OUTPUT);
    pinMode(UNLOCK_PIN, OUTPUT);
    delay(10);
    ValidRFID1 = "108792ab";
    ValidRFID2 = "5a9092ab";
    ValidRFID3 = "168a8bab";

    //Serial.print("Valid card id: ");
    //dump_byte_array(ValidRFID1, 4);
    //Serial.print(ValidRFID1);
    //Serial.println();

}

void loop() {
  // put your main code here, to run repeatedly:

    digitalWrite(SS_PIN1, LOW);
    board1Valid = check_card(board1);
    digitalWrite(SS_PIN1, HIGH);

    digitalWrite(SS_PIN2, LOW);
    board2Valid = check_card(board2);
    digitalWrite(SS_PIN2, HIGH);

    digitalWrite(SS_PIN3, LOW);
    board3Valid = check_card(board3);
    digitalWrite(SS_PIN3, HIGH);

    //digitalWrite(UNLOCK_PIN, HIGH);
    Serial.println(String(board1Valid, BIN) + String(board2Valid, BIN) + String(board3Valid, BIN));

    //Check if we meet the unlock requirements
    check_unlock();

    //Do something when unlocked!
    if (unlocked == 1) {
      digitalWrite(UNLOCK_PIN, HIGH);
    }
    else {
      digitalWrite(UNLOCK_PIN, LOW);
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
    
    // Grab the serail off the card 
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
void check_unlock(){
  if (board1Valid == 1 && board2Valid == 1 && board3Valid == 1) {
    Serial.println("Unlocked!");
    unlocked = 1;
  }
  else {
    unlocked = 0;
  }
}



