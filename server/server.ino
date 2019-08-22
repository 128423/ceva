#include <ESPmDNS.h>
#include <IOXhop_FirebaseESP32.h>
#include <NTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define FIREBASE_HOST "https://cervejaproject.firebaseio.com/"
#define FIREBASE_AUTH "wUVkY1GOhjqga45jdyu7CHG3k8jrja68KWq2TM1n"

// model para guardar a temperatura no database1
struct temp {
  String tmp;
  String data;
};

// constantes
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
StaticJsonBuffer<200> jsonBuffer;
WebServer server(80);

const char *ssid = "Luis_Ap";
const char *password = "95370000";

int ant1 = 0;
int ant2 = 0;
int ant3 = 0;
int ant4 = 0;
int ant5 = 0;

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  char temp[3000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(
      temp, 3000,
      "<html><title>Ceva Project</title><meta charset=\"UTF-8\" /><meta "
      "name=\"viewport\" content=\"width=device-width, initial-scale=1\" "
      "/><link rel=\"stylesheet\" "
      "href=\"https://www.w3schools.com/w3css/4/w3.css\" /><body "
      "class=\"w3-content\" style=\"max-width:1300px\">  <!-- First Grid: Logo "
      "& About -->  <div class=\"w3-row\">    <div class=\"w3-half w3-black "
      "w3-container w3-center\" style=\"height:700px\">      <div "
      "class=\"w3-padding-64\">        <h1>modo</h1>      </div>      <div "
      "class=\"w3-padding-64\">        <button class=\"w3-button w3-black "
      "w3-block w3-hover-brown w3-padding-16\">          30 sec        "
      "</button>        <button class=\"w3-button w3-black w3-block "
      "w3-hover-brown w3-padding-16\">          1 min        </button>        "
      "<button class=\"w3-button w3-black w3-block w3-hover-brown "
      "w3-padding-16\">          5 min        </button>        <button "
      "class=\"w3-button w3-black w3-block w3-hover-brown w3-padding-16\">     "
      "     10 min        </button>        <button class=\"w3-button w3-black "
      "w3-block w3-hover-brown w3-padding-16\">          Reset Time        "
      "</button>      </div>    </div>    <div class=\"w3-half w3-blue-grey "
      "w3-container\" style=\"height:700px\">      <div class=\"w3-padding-64 "
      "w3-center\">        <h1>Ultima Consulta</h1>        <h2>temp: <b> "
      "69</b></h2>        <div class=\"w3-left-align w3-padding-large\">       "
      "   <div id=\"piechart\"></div>          <script "
      "type=\"text/javascript\" "
      "src=\"https://www.gstatic.com/charts/loader.js\"></script>          "
      "<script type=\"text/javascript\">            "
      "google.charts.load(\"current\", { packages: [\"corechart\"] });         "
      "   google.charts.setOnLoadCallback(drawChart);            function "
      "drawChart() {              var data = "
      "google.visualization.arrayToDataTable([                [\"tempo\", "
      "\"Temperatura\"],                [\"time1\", 8],                "
      "[\"time2\", 2],                [\"time3\", 4],                "
      "[\"time4\", 2],                [\"time5\", 60]              ]);         "
      "     var options = {                title: \"ultimas 5 mediçoes\",      "
      "          width: 550,                height: 400              };        "
      "      var chart = new google.visualization.LineChart(                "
      "document.getElementById(\"piechart\")              );              "
      "chart.draw(data, options);            }          </script>        "
      "</div>      </div>    </div>  </div>  <!-- Footer -->  <footer "
      "class=\"w3-container w3-black w3-padding-16\">    <p>Batata</p>  "
      "</footer></body></html>",
      hr, min % 60, sec % 60);

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
  server.on("/test.svg", drawGraph);
  server.on("/inline",
            []() { server.send(200, "text/plain", "this works as well"); });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  timeClient.begin();
  timeClient.setTimeOffset(-10800);
  temp tmp = getTemperatura();
  ConnectWithDatabase(tmp);
}

void loop(void) { server.handleClient(); }

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
         "width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" "
         "stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(
        temp,
        "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n",
        x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

temp getTemperatura() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  String formattedDate = timeClient.getFormattedDate();
  temp tmp;
  tmp.tmp = "69";
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
