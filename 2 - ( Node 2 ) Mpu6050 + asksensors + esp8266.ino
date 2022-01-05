#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


Adafruit_MPU6050 mpu;
WiFiClientSecure client;


const char* wifi_ssid = "IoT Maker CO-Work";             // SSID
const char* wifi_password = "12345678k";         // WIFI
const char* apiKeyIn = "wQVtwrxM4FvKDAIa1bveX3TY1MDF0Qxo";      // API KEY IN
const unsigned int writeInterval = 200; // write interval (in ms)

// ASKSENSORS config.
const char* https_host = "api.asksensors.com";         // ASKSENSORS host name
const int https_port = 443;                        // https port
const char* https_fingerprint =  "B5 C3 1B 2C 0D 5D 9B E5 D6 7C B6 EF 50 3A AD 3F 9F 1E 44 75";     // ASKSENSORS HTTPS SHA1 certificate


void setup(void) {
  Serial.begin(115200);

  //================ connecting start ================//

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

  //================ connecting end ==================//

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // set accelerometer range to +-8G
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // set gyro range to +- 500 deg/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // set filter bandwidth to 21 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
  client.setInsecure();

}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s"); // 9.4 rad = 1.5 cycle
  Serial.println("");
  //------------------------------------------------
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("********** connecting to HOST : ");
  Serial.println(https_host);
  if (!client.connect(https_host, https_port)) {
    Serial.println("-> connection failed");
    //return;
  }


  // Create a URL for the request
  
  String url = "/write/";
  url += apiKeyIn;
  url += "?module1=";
  url += a.acceleration.x;
  url += "&module2=";
  url += a.acceleration.y;
  url += "&module3=";
  url +=  a.acceleration.z;

  //---------------------------
  
  url += "&module4=";
  url +=  g.gyro.x;
  url += "&module5=";
  url +=  g.gyro.y;
  url += "&module6=";
  url +=  g.gyro.z;

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

  delay(writeInterval );     // delay in msec
}
