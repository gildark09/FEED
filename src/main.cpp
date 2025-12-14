#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

ESP8266WebServer server(80); 

String savedSSID = "";
String savedPASS = "";

Servo feeder;

#define SERVO_PIN D5      // GPIO14
#define TRIG_PIN  D6      // GPIO12
#define ECHO_PIN  D7      // GPIO13
#define GREEN D1
#define RED D2
const int containerHeight = 25;

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

void handleRoot();              
void sendOK();
void handleFeed();
void handleSave();  
void handleLevel();

long getStableDistance() {
  long sum = 0;
  int count = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    long distance = duration * 0.034 / 2;

    if (distance > 2 && distance < 180) {
      sum += distance;
      count++;
    }

    delay(50);
    Serial.print("Raw reading: ");
    Serial.println(distance);
    }

  if (count == 0) return -1;
  return sum / count;
}



void setup() {
  Serial.begin(115200);
  feeder.attach(SERVO_PIN);

  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Start AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP_Feeder_Setup", "12345678");

  Serial.println("\nAP Started");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP()); // 192.168.4.1

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/feed", handleFeed);
  server.on("/level", handleLevel);

  feeder.write(0);

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
  delay(1000);

  digitalWrite(GREEN, HIGH);
  feeder.write(90);    
  delay(2000);
  feeder.write(0);
  digitalWrite(GREEN, LOW);

  Serial.println("NIGANA MAN!");
} 

void handleLevel() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  long distance = getStableDistance();
  int level = 0;

  Serial.print("Filtered distance: ");
  Serial.println(distance);

  if (distance > 0) {
    level = map(distance, 0, containerHeight, 100, 0);
    level = constrain(level, 0, 100);
  }

  if(level < 20){
    digitalWrite(RED, HIGH);
    delay(500);
    digitalWrite(RED, LOW);
  }

  String json = "{\"level\":" + String(level) + "}";
  server.send(200, "application/json", json);

  Serial.printf("Food Level: %d%%\n", level);
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