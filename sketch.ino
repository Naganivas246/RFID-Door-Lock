#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN   10
#define RST_PIN   9
#define GREEN_LED 4
#define RED_LED   5
#define BUZZER    7
#define SERVO_PIN 6

MFRC522 rfid(SS_PIN, RST_PIN);
Servo lockServo;

// --- Add your authorised card UIDs here ---
String authorisedUIDs[] = {
  "A1 B2 C3 D4",   // Card 1
  "12 34 56 78"    // Fob 1
};
int numCards = 2;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lockServo.attach(SERVO_PIN);
  lockServo.write(0);          // Start locked
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.println("Ready — scan a card...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.trim();
  uid.toUpperCase();
  Serial.println("Card UID: " + uid);

  if (isAuthorised(uid)) {
    grantAccess();
  } else {
    denyAccess();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool isAuthorised(String uid) {
  for (int i = 0; i < numCards; i++) {
    if (uid == authorisedUIDs[i]) return true;
  }
  return false;
}

void grantAccess() {
  Serial.println("ACCESS GRANTED");
  digitalWrite(GREEN_LED, HIGH);
  tone(BUZZER, 1000, 200);
  lockServo.write(90);          // Unlock
  delay(3000);                  // Hold open 3 seconds
  lockServo.write(0);           // Relock
  digitalWrite(GREEN_LED, LOW);
}

void denyAccess() {
  Serial.println("ACCESS DENIED");
  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 400, 150);
    delay(200);
    digitalWrite(RED_LED, LOW);
    delay(150);
  }
}
