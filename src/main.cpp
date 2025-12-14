#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

ESP8266WebServer server(80); 

String savedSSID = "";
String savedPASS = "";

Servo feeder;

#define SERVO_PIN D4


const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP Feeder Setup</title>
</head>
<body>
  <h2>WiFi Setup</h2>
  <form action="/save">
    SSID:<br>
    <input type="text" name="ssid"><br>
    Password:<br>
    <input type="password" name="pass"><br><br>
    <input type="submit" value="Connect">
  </form>
</body>
</html>
)rawliteral";

void handleRoot();              // function prototypes for HTTP handlers
void sendOK();
void handleFeed();
void handleSave();  


void setup() {
  Serial.begin(115200);
  feeder.attach(SERVO_PIN);

  // Start AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP_Feeder_Setup", "12345678");

  Serial.println("\nAP Started");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // 192.168.4.1

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/feed", handleFeed);

  server.begin();
  Serial.println("HTTP server started");
}

/* ---------- LOOP ---------- */

void loop() {
  server.handleClient();
}


void handleRoot() {
  server.send(200, "text/html", htmlPage);
}


void handleFeed() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Fed successfully");

  feeder.write(90);
  delay(1500);
  feeder.write(0);

  Serial.println("SUCCESS!");
} 


void handleSave() {
  savedSSID = server.arg("ssid");
  savedPASS = server.arg("pass");

  server.send(200, "text/plain", "Connecting to WiFi...");

  Serial.println("Received credentials:");
  Serial.println(savedSSID);
  Serial.println(savedPASS);

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPASS.c_str());

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}