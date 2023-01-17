#include <WiFi.h>
#include <FirebaseESP32.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Rindu Alam"
#define WIFI_PASSWORD "90949699"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCUPOcfJgv5wGwDlY-U0d4KelZ4by7nR7c"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp32-autowatering-system-default-rtdb.asia-southeast1.firebasedatabase.app" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int value =0;
bool signupOK = false;

#define RELAY    16 // ESP32 pin GIOP17 that connects to relay
#define MOISTURE_PIN 36 // ESP32 pin GIOP36 (ADC0) that connects to AOUT pin of moisture sensor

#define THRESHOLD 3000 // CHANGE YOUR THRESHOLD HERE

void setup() {
  pinMode(RELAY, OUTPUT);
  // pinMode(MOISTURE_PIN, INPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    int value = analogRead(MOISTURE_PIN);
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "temperatur/nilai", value)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (value < THRESHOLD) {

      Serial.print("The soil is WET => turn pump OFF");
      digitalWrite(RELAY, LOW);
    
    } else {
      Serial.print("The soil is DRY => turn pump ON");
      digitalWrite(RELAY, HIGH);
    }

    Serial.print(" (");
    Serial.print(value);
    Serial.println(")");

    delay(1000);

  // //FIREBASE
  // Firebase.RTDB.setInt(&fbdo, "temperatur/nilai", value);
  }
}
