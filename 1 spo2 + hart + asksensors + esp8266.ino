#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DFRobot_MAX30102.h>
DFRobot_MAX30102 particleSensor;

int x = 1, y = 2, z = 3;
// user config: TODO

const char* wifi_ssid = "IoT Maker CO-Work";             // SSID
const char* wifi_password = "12345678k";         // WIFI
const char* apiKeyIn = "NuwfrrgRimNidDONF2LnkcsZxm6KooXI";      // API KEY IN
const unsigned int writeInterval = 200; // write interval (in ms)

// ASKSENSORS config.
const char* https_host = "api.asksensors.com";         // ASKSENSORS host name
const int https_port = 443;                        // https port
const char* https_fingerprint =  "B5 C3 1B 2C 0D 5D 9B E5 D6 7C B6 EF 50 3A AD 3F 9F 1E 44 75";     // ASKSENSORS HTTPS SHA1 certificate


uint32_t delayMS;
int status = WL_IDLE_STATUS;
WiFiClientSecure client;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("********** connecting to WIFI : ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> WiFi connected");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());
  // Set Client to insecure
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 was not found");
    delay(1000);
  }
  particleSensor.sensorConfiguration(/*ledBrightness=*/50, /*sampleAverage=*/SAMPLEAVG_4, \
      /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_100, \
      /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);

  client.setInsecure();
}

int32_t SPO2; //SPO2
int8_t SPO2Valid; //Flag to display if SPO2 calculation is valid
int32_t heartRate; //Heart-rate
int8_t heartRateValid; //Flag to display if heart-rate calculation is valid

void loop() {

  particleSensor.heartrateAndOxygenSaturation(/**SPO2=*/&SPO2, /**SPO2Valid=*/&SPO2Valid, /**heartRate=*/&heartRate, /**heartRateValid=*/&heartRateValid);
  //Print result
  Serial.print(F("heartRate="));
  Serial.print(heartRate, DEC);
  Serial.print(F(", heartRateValid="));
  Serial.print(heartRateValid, DEC);
  Serial.print(F("; SPO2="));
  Serial.print(SPO2, DEC);
  Serial.print(F(", SPO2Valid="));
  Serial.println(SPO2Valid, DEC);


  // Use WiFiClientSecure class to create TLS connection
  Serial.print("********** connecting to HOST : ");
  Serial.println(https_host);
  if (!client.connect(https_host, https_port)) {
    Serial.println("-> connection failed");
    //return;
  }
  if ((heartRate < 40 ) || (heartRate > 120 )) {
    heartRate = 0 ;
  }

  if ((SPO2 < 40) || (SPO2 > 101)) {
    SPO2 = 0 ;
  }


  String url = "/write/";
  url += apiKeyIn;
  url += "?module1=";
  url += SPO2;
  url += "&module2=";
  url += heartRate;




  Serial.print("********** requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + https_host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("> Request sent to ASKSENSORS");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      String line = client.readStringUntil('\n');
      Serial.println("********** ASKSENSORS replay:");
      Serial.println(line);
      Serial.println("********** closing connection");

      break;
    }
  }

  delay(writeInterval);     // delay in msec
}
