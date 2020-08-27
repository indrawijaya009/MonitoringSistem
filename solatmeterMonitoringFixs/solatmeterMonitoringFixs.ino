#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;

const char* ssid     = "RemoteXY";
const char* password = "remote456";

PZEM004Tv30 pzem(D5, D6);  // (RX,TX) connect to TX,RX of PZEM
const int RELAY_PIN = D1;

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  Serial.print("Connecting");
  // Wait for connection
  pinMode(RELAY_PIN, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    delay(250);
  }
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("Connected to Network/SSID");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

}

void loop() {
  // wait for WiFi connection
  //main energy meter
  HTTPClient http; //Declare objeck of class HTTPClient
  String postData, volt, amper, watt, kwh, frequency, powerfactor;
  float v = pzem.voltage();
  float a = pzem.current();
  float w = pzem.power();
  float kw = pzem.energy();
  float f = pzem.frequency();
  float pft = pzem.pf();
  volt = String(v);
  amper = String(a);
  watt = String(w);
  kwh = String(kw);
  frequency = String(f);
  powerfactor = String(pft);

  postData =  "volt=" + volt + "&amper=" + amper + "&watt=" + watt + "&kwh=" + kwh  + "&suhu=" + frequency + "&kelembaban=" + powerfactor;
  http.begin("http://db.solarmeter.id/user/1?");//api local
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
//    Serial.println(httpCode);   //Print HTTP return code
//    Serial.println(payload);    //Print request response payload
//    Serial.println(postData);
  updaterilay();
  delay(10000);

}

void updaterilay()
{

  HTTPClient http;    //Declare object of class HTTPClient

  //  Serial.print("Request Link:");

  http.begin("http://db.solarmeter.id/user/1");     //Specify request destination

  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload from server

  Serial.print("Response Code:"); //200 is OK
  Serial.println(httpCode);   //Print HTTP return code

  Serial.print("Returned data from Server:");
  Serial.println(payload);    //Print request response payload

  if (httpCode == 200)
  {
    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    DynamicJsonBuffer jsonBuffer(capacity);

    // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println(F("Parsing failed!"));
      return;
    }

    // Decode JSON/Extract values
    Serial.println(F("Response:"));
    String nilai = (root["nilai"].as<char*>());
    //    const char* device = root[0]["nilai"];
    //    String
    Serial.println(nilai);
    if (nilai == "1") {
      digitalWrite(RELAY_PIN, LOW);
    }
    else {
      digitalWrite(RELAY_PIN, HIGH);
    }

  }
  else
  {
    Serial.println("Error in response");
  }

  http.end();  //Close connection

  delay(5000);  //GET Data at every 5 seconds

}
