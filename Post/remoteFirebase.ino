#include <DallasTemperature.h>
#include <ESPmDNS.h>
#include <IOXhop_FirebaseESP32.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define TIMMER_TO_POST 300000
#define FIREBASE_HOST "https://cervejaproject.firebaseio.com/"
#define FIREBASE_AUTH "wUVkY1GOhjqga45jdyu7CHG3k8jrja68KWq2TM1n"
const char *ssid = "sbsistemas_colaboradores";
const char *password = "sbsistemas13524500";
const int led = 13;
WiFiUDP ntpUDP;
StaticJsonBuffer<200> jsonBuffer;
NTPClient timeClient(ntpUDP);
OneWire oneWire(22);
DallasTemperature tempSensor(&oneWire);

struct temp {
  float tmp;
  String data;
};

temp getTemperatura() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  String formattedDate = timeClient.getFormattedDate();
  tempSensor.requestTemperaturesByIndex(0);
  temp tmp;
  tmp.tmp = tempSensor.getTempCByIndex(0);
  tmp.data = formattedDate;
  return tmp;
}

void ConnectWithDatabase(temp tmp) {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("/ip", WiFi.localIP().toString());
  JsonObject &root = jsonBuffer.createObject();
  root["temperatura"] = tmp.tmp;
  root["data"] = tmp.data;
  Firebase.push("/temp", root);
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  timeClient.begin();
  timeClient.setTimeOffset(-10800);
  tempSensor.begin();
}

void loop(void) {
  temp tmp = getTemperatura();
  ConnectWithDatabase(tmp);
  sleep(TIMMER_TO_POST);
}