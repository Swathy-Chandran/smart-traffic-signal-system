// Gas Sensor Monitoring using ESP32,MQ-135 and Firebase
// Author: Swathy Chandran N

#include <WiFi.h>
#include "Firebase_ESP_Client.h"

// WiFi credentials (replace with your own)
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

// Firebase configuration
#define API_KEY "AIzaSyDS7NFjMLgP7TrGyc1A7N3zsSWDGpR7GtA"
#define DATABASE_URL "https://traffic-d9a20-default-rtdb.firebaseio.com"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Pins
int gasPin = 34;
int red_led = 32;
int green_led = 33;
int connection_led = 2;

// Variables
int gas_threshold = 150;
String gas_status = "ABSENT";
String dataPath = "GasMonitoring";

void setup()
{
    Serial.begin(115200);

    pinMode(gasPin, INPUT);
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
    pinMode(connection_led, OUTPUT);

    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);
    digitalWrite(connection_led, HIGH);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Firebase setup
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("\nFirebase Sign-up/Login Success (Anonymous)");
    }
    else
    {
        Serial.printf("\nFirebase Sign-up/Login Failed: %s\n",
                      config.signer.signupError.message.c_str());
    }

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop()
{
    int gasValue = analogRead(gasPin);

    // Check pollution level
    if (gasValue > gas_threshold)
    {
        gas_status = "PRESENT";
        digitalWrite(red_led, HIGH);
        digitalWrite(green_led, LOW);
    }
    else
    {
        gas_status = "ABSENT";
        digitalWrite(red_led, LOW);
        digitalWrite(green_led, HIGH);
    }

    // Print data
    Serial.print("GAS: ");
    Serial.print(gasValue);
    Serial.print(", POLLUTION: ");
    Serial.println(gas_status);

    // Send data to Firebase
    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(connection_led, HIGH);

        if (Firebase.ready())
        {
            Firebase.RTDB.setInt(&fbdo, dataPath + "/gas", gasValue);
            Firebase.RTDB.setString(&fbdo, dataPath + "/pollution", gas_status);
            Firebase.RTDB.pushInt(&fbdo, dataPath + "/gas_history", gasValue);
        }
    }
    else
    {
        digitalWrite(connection_led, LOW);

        WiFi.disconnect();
        Serial.print("Reconnecting to WiFi: ");
        Serial.println(ssid);

        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
    }

    delay(100);
}
