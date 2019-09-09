#include <DallasTemperature.h>
#include <ESPmDNS.h>
#include <IOXhop_FirebaseESP32.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define FIREBASE_HOST "https://cervejaproject.firebaseio.com/"
#define FIREBASE_AUTH "wUVkY1GOhjqga45jdyu7CHG3k8jrja68KWq2TM1n"

// model para guardar a temperatura no database1
struct temp {
  float tmp;
  String data;
};

// constantes
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
StaticJsonBuffer<200> jsonBuffer;
WebServer server(80);
OneWire oneWire(22);
DallasTemperature tempSensor(&oneWire);

const char *ssid = "sbsistemas_colaboradores";
const char *password = "sbsistemas13524500";

int timmer = 10;
float ant1 = 0;
float ant2 = 0;
float ant3 = 0;
float ant4 = 0;
float ant5 = 0;

/**
 *  Da Push no Grafico
 */
void termpush(float aux) {
  ant5 = ant4;
  ant4 = ant3;
  ant3 = ant2;
  ant2 = ant1;
  ant1 = aux;
}

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  char temp[3000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(
      temp, 3000,
      "<html>  <title>Ceva Project</title>  <meta charset=\"UTF-8\" />  <meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1\" />  "
      "<link rel=\"stylesheet\" "
      "href=\"https://www.w3schools.com/w3css/4/w3.css\" />  <body "
      "class=\"w3-content\" style=\"max-width:1300px\">    <!-- First Grid: "
      "Logo & About -->    <div class=\"w3-row\">      <div class=\"w3-half "
      "w3-black w3-container w3-center\" style=\"height:700px\">        <div "
      "class=\"w3-padding-64\">          <h1>modo</h1>        </div>        "
      "<div class=\"w3-padding-64\">          <button            "
      "class=\"w3-button w3-black w3-block w3-hover-brown w3-padding-16\"      "
      "    >            30 sec          </button>          <button            "
      "class=\"w3-button w3-black w3-block w3-hover-brown w3-padding-16\"      "
      "    >            1 min          </button>          <button            "
      "class=\"w3-button w3-black w3-block w3-hover-brown w3-padding-16\"      "
      "    >            5 min          </button>          <button            "
      "class=\"w3-button w3-black w3-block w3-hover-brown w3-padding-16\"      "
      "    >            10 min          </button>          <button            "
      "class=\"w3-button w3-black w3-block w3-hover-brown w3-padding-16\"      "
      "    >            Reset Time          </button>        </div>      "
      "</div>      <div class=\"w3-half w3-blue-grey w3-container\" "
      "style=\"height:700px\">        <div class=\"w3-padding-64 w3-center\">  "
      "        <h1>Ultima Consulta</h1>          <h2>            temp:         "
      "   <b> %f </b>          </h2>          <div class=\"w3-left-align "
      "w3-padding-large\">            <div id=\"piechart\" style=\"width: "
      "300px; height: 300px;\"></div>            <script              "
      "type=\"text/javascript\"              "
      "src=\"https://www.gstatic.com/charts/loader.js\"            ></script>  "
      "          <script type=\"text/javascript\">              "
      "google.charts.load(\"current\", { packages: [\"corechart\"] });         "
      "     google.charts.setOnLoadCallback(drawChart);              function "
      "drawChart() {                var data = "
      "google.visualization.arrayToDataTable([                  [\"tempo\", "
      "\"Temperatura\"],                  [\"temp5\", %f],                  "
      "[\"temp4\", %f],                  [\"temp3\", %f],                  "
      "[\"temp2\", %f],                  [\"temp1\", %f]                ]);    "
      "            var options = {                  title: \"ultimas 5 "
      "mediçoes\",                };                var chart = new "
      "google.visualization.AreaChart(                  "
      "document.getElementById(\"piechart\")                );                "
      "chart.draw(data, options);              }            </script>          "
      "</div>        </div>      </div>    </div>    <!-- Footer -->    "
      "<footer class=\"w3-container w3-black w3-padding-16\">      "
      "<p>Batata</p>    </footer>  </body></html>",
      ant1, ant5, ant4, ant3, ant2, ant1);

  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
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

  server.on("/", handleRoot);
  server.on("/time/{}", []() {
    String aux = server.pathArg(0);
    // timmer = atoi(aux);
    server.send(200, "text/plain", "timer: '" + aux + "'");
  });
  server.on("/inline",
            []() { server.send(200, "text/plain", "this works as well"); });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  timeClient.begin();
  timeClient.setTimeOffset(-10800);
  tempSensor.begin();
  temp tmp = getTemperatura();
  ConnectWithDatabase(tmp);
}

void loop(void) { server.handleClient(); }

temp getTemperatura() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  String formattedDate = timeClient.getFormattedDate();
  tempSensor.requestTemperaturesByIndex(0);
  temp tmp;
  tmp.tmp = tempSensor.getTempCByIndex(0);
  termpush(tmp.tmp);
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
