#include <SoftwareSerial.h>

// Alarm receiver's phone numbers with country code
const String PHONE_NUMBERS[] = {"+989120000000", "+989030000000", "+1954000000"}; // Replace with actual numbers
const int NUMBER_COUNT = sizeof(PHONE_NUMBERS) / sizeof(PHONE_NUMBERS[0]);

//GSM Module RX pin to Arduino 3
//GSM Module TX pin to Arduino 2
#define rxPin 2
#define txPin 3
SoftwareSerial sim800(rxPin,txPin);

//the pin that the pir sensor is attached to
int pir_sensor1 = 11;
int pir_sensor2 = 12; // Second PIR sensor on pin 12

// Define message strings
const String MOTION_DETECTED_MESSAGE1 = "Motion detected by PIR sensor 1 (Front door)! Check the area.";
const String MOTION_DETECTED_MESSAGE2 = "Motion detected by PIR sensor 2 (Darkzed room)! Check the area.";
const String POWER_FAILURE_MESSAGE = "Power failure detected! Check power source."; // Message for power failure
const String POWER_RESTORED_MESSAGE = "Power restored!"; // Message for power restored
const String SYSTEM_ARMED_MESSAGE = "Alarm system armed and active!"; // Message when the system starts

// Define Relay Pin
#define relayPin 8 // Digital pin connected to the relay module

// Define duration for relay activation (in milliseconds)
#define RELAY_DURATION 180000 // 3 minutes = 180,000 milliseconds  (Changed from 600000 to 180000)

// Define Buzzer Pin
#define buzzerPin 9 // Digital pin connected to the buzzer

// Define Power Detection Pin
#define powerDetectPin 13 // Digital pin to detect power status

// Flags
bool alertSent = false; // Flag to prevent sending SMS/Call more than once in short time
bool powerFailureDetected = false; // Flag to track if power failure has already been reported
bool powerRestoredDetected = false; // Flag to track if power has been restored
bool systemArmedSent = false; // Flag to track if "system armed" SMS has been sent

unsigned long relayStartTime = 0;  // Variable to store the time when the relay was activated

void setup() {
  pinMode(pir_sensor1, INPUT);    // initialize sensor1 as an input
  pinMode(pir_sensor2, INPUT);    // initialize sensor2 as an input

  pinMode(relayPin, OUTPUT);   // Initialize the relay pin as an output
  digitalWrite(relayPin, LOW);  // Ensure the relay is initially off

  pinMode(buzzerPin, OUTPUT); // Initialize the buzzer pin as an output
  digitalWrite(buzzerPin, LOW); // Ensure the buzzer is initially off

  pinMode(powerDetectPin, INPUT_PULLUP); // Initialize the power detection pin as an input with pull-up resistor

  Serial.begin(115200);          // initialize serial

  // Delay for 10 seconds (10000 milliseconds)
  Serial.println("Waiting 10 seconds for modules to initialize...");
  delay(10000);
  Serial.println("Initialization delay complete.");

  sim800.begin(9600);
  Serial.println("SIM800L software serial initialize");

  sim800.println("AT");
  delay(1000);

  // Configure SIM800L for Iranian Mobile Operator (Hamrahe Aval)
  sim800.println("AT+CMGF=1"); // Set SMS format to text mode
  delay(100);
  sim800.println("AT+CSCS=\"GSM\""); // Set character set to GSM
  delay(100);
  sim800.println("AT+CLIP=1");
  delay(100);
  sim800.println("AT+CNMI=2,2,0,0,0");
  delay(100);
  Serial.println("SIM800L configured for Hamrahe Aval");

    // Send "system armed" SMS at startup
    delay(5000); // Give SIM800L some time to initialize
    Serial.println("Sending System Armed SMS...");
    for (int i = 0; i < NUMBER_COUNT; i++) {
        Serial.print("Sending SMS to: ");
        Serial.println(PHONE_NUMBERS[i]);
        sendSMS(PHONE_NUMBERS[i], SYSTEM_ARMED_MESSAGE);
        delay(5000); // Wait a bit before sending SMS to the next number
    }
    Serial.println("Finished sending System Armed SMS.");
    systemArmedSent = true;
}

void loop(){

//////////////////////////////////////////////////
while(sim800.available()){
  Serial.println(sim800.readString());
}
//////////////////////////////////////////////////
while(Serial.available())  {
  sim800.println(Serial.readString());
}
//////////////////////////////////////////////////
  int val1 = digitalRead(pir_sensor1);  // read sensor1 value
  int val2 = digitalRead(pir_sensor2);  // read sensor2 value
  String messageToSend = "";

  // Check for motion on either sensor and if SMS/Call hasn't been sent recently
  if (!alertSent) {
      if (val1 == HIGH) {
        Serial.println("Motion detected by PIR sensor 1 (Pin 11)!");
        messageToSend = MOTION_DETECTED_MESSAGE1;
      } else if (val2 == HIGH) {
        Serial.println("Motion detected by PIR sensor 2 (Pin 12)!");
        messageToSend = MOTION_DETECTED_MESSAGE2;
      }

      if(val1 == HIGH || val2 == HIGH) {
          //First, Send SMS to all numbers in the list
          Serial.println("Sending SMS to all numbers...");
          for (int i = 0; i < NUMBER_COUNT; i++) {
            Serial.print("Sending SMS to: ");
            Serial.println(PHONE_NUMBERS[i]);
            sendSMS(PHONE_NUMBERS[i], messageToSend);
            delay(5000); // Wait a bit before sending SMS to the next number
          }
          Serial.println("Finished sending SMS to all numbers.");

          // Activate Relay
          Serial.println("Activating Relay...");
          digitalWrite(relayPin, HIGH); // Turn the relay ON
          relayStartTime = millis();   // Record the time when the relay was turned ON

          // Activate Buzzer
          Serial.println("Activating Buzzer...");
          tone(buzzerPin, 1000); // Start tone (frequency 1000 Hz)

          // Call to all numbers in the list
          Serial.println("Calling all numbers...");
          for (int i = 0; i < NUMBER_COUNT; i++) {
            Serial.print("Calling: ");
            Serial.println(PHONE_NUMBERS[i]);
            callNumber(PHONE_NUMBERS[i]);
            delay(25000); // Wait a bit before calling the next number
          }
        Serial.println("Finished calling all numbers in the list.");

        alertSent = true;  // Set the flag to indicate SMS/Call was sent
    }

  }

    // Check if the relay has been active for RELAY_DURATION
    if (alertSent && (millis() - relayStartTime >= RELAY_DURATION)) {
        Serial.println("Deactivating Relay and Buzzer...");
        digitalWrite(relayPin, LOW);   // Turn the relay OFF
        noTone(buzzerPin);    // Turn off the buzzer
        alertSent = false;     // Reset alert flag
    }

    // Power Failure Detection
    int powerStatus = digitalRead(powerDetectPin);

    // Power Failure Detection
    if (powerStatus == HIGH && !powerFailureDetected) { // HIGH means disconnected from GND
      Serial.println("Power failure detected!");
      // Send power failure SMS to ALL numbers in the list
      for (int i = 0; i < NUMBER_COUNT; i++) {
        sendSMS(PHONE_NUMBERS[i], POWER_FAILURE_MESSAGE);
        delay(5000); // Wait a bit before sending SMS to the next number
      }
      powerFailureDetected = true; // Set the flag to prevent repeated SMS messages
      powerRestoredDetected = false; // Reset the restored flag, so we can send that SMS later
    }

    // Power Restored Detection
    if (powerStatus == LOW && powerFailureDetected && !powerRestoredDetected) { // LOW means connected to GND
      Serial.println("Power restored!");
      // Send power restored SMS to ALL numbers in the list
      for (int i = 0; i < NUMBER_COUNT; i++) {
        sendSMS(PHONE_NUMBERS[i], POWER_RESTORED_MESSAGE);
        delay(5000); // Wait a bit before sending SMS to the next number
      }
      powerRestoredDetected = true; // Set the flag to prevent repeated restored SMS messages
      powerFailureDetected = false; // Reset the failure flag, so we can send that SMS later
    }
}

// Function to send SMS
void sendSMS(String phoneNumber, String message) {
  sim800.println("AT+CMGS=\"" + phoneNumber + "\""); // Command to send SMS
  delay(100);
  sim800.print(message); // The message
  delay(100);
  sim800.println((char)26); // End AT command with a ^Z, ASCII code 26
  delay(10000); // Give module time to send SMS
  Serial.println("SMS Sent");
}

// Function to call a number
void callNumber(String phoneNumber) {
  sim800.print("ATD");
  sim800.print(phoneNumber);
  sim800.println(";");
  delay(20000); // Allow the call to connect
  sim800.println("ATH"); // Hang up
  Serial.println("Call Ended");
}