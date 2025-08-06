#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "SystemLogic.h"
#include "DHT.h"
#define DHTTYPE DHT11
// Pin definitions
const int DHT_PIN = 0;
const int PHOTOCELL_PIN = 32;
const int SOIL_PIN = 39;
const int SERVO_PIN = 2;

// WiFi credentials
const char* WIFI_NAME = "Stevens-IoT";
const char* WIFI_PASS = "nMN882cmg7";

// MQTT configuration
const char* MQTT_SERVER = "98db5050a791439c98eac188febfecbe.s2.eu.hivemq.cloud";
const char* MQTT_USER = "stevens";
const char* MQTT_PASS = "Stevens@1870";
const int MQTT_PORT = 8883;

// Team information
const String YEAR = "2023";
const String CLASS = "ENGR111";
const String SECTION = "C";
const String GROUP_NUMBER = "N1";
String PAYLOAD;

// Sensor configuration
const int SOIL_MAXWET = 1130;
const int SOIL_MAXDRY = 3300;

const int SERVO_OPEN_ANGLE = 90;
const int SERVO_CLOSE_ANGLE = 0;

int pos = 0;


void setup() {
  wifi_start();
  mqtt_start();
  enable_sensors();
  pinMode(0, DHTTYPE);

}

const int publishInterval = 1000;

void loop() {
  mqtt_loop();



  int photo = analogRead(PHOTOCELL_PIN);
  Serial.print("Photocell Value: ");
  int lux = 1 / pow((((3.3 / (3.3 / 4096 * photo)) * 10000) - 10000) / (8 * pow(10, 6)), 0.817);
  int soilhumidity = analogRead(SOIL_PIN);
  int soilPercent = map(soilhumidity, SOIL_MAXWET, SOIL_MAXDRY, 100, 0);
  Serial.print("Soil Humidity Reading: ");
  Serial.print(soilPercent);
  Serial.println(" %");
  int temp = dht.readTemperature();
  int hum = dht.readHumidity();
  if(soilPercent <= 30){
    for(pos = 0; pos<=90;pos+=1){
      myservo.attach(SERVO_PIN, 500, 2500);  // attaches the servo on pin 2 to the servo object
      myservo.write(pos);
      delay(15);
    }
    delay(1000);
    for(pos = 90; pos>=0;pos-=1){
      myservo.attach(SERVO_PIN, 500, 2500);  // attaches the servo on pin 2 to the servo object
      myservo.write(pos);
      delay(15);
    }
  }
  char pubString[8];
  dtostrf(soilPercent, 1, 2, pubString);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String soilTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "soil_hum";
  client.publish(soilTopic.c_str(), pubString);
  delay(publishInterval);  // 100secs delay for publishing
  dtostrf(lux, 1, 2, pubString);
  String luxTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "lux";
  client.publish(luxTopic.c_str(), pubString);
  dtostrf(temp, 1, 2, pubString);
  String tempTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "temp";
  client.publish(tempTopic.c_str(), pubString);
  dtostrf(hum, 1, 2, pubString);
  String humTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "hum";
  client.publish(humTopic.c_str(), pubString);
  myservo.detach();
}
