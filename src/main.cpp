#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

ESP8266WebServer server(80); 

const char* ssid = "Waypii";
const char* password = "@TANfamilyasd456";

Servo feeder;

#define SERVO_PIN D4


void handleRoot();              // function prototypes for HTTP handlers
void sendOK();
void handleFeed();



void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  feeder.attach(SERVO_PIN);
  WiFi.begin(ssid, password);

  Serial.println("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.println("\nESP8266 IP:");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  
  server.on("/feed", handleFeed);
  
  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

  

}



void loop(void){
  server.handleClient();                    // Listen for HTTP requests from clients
}

void sendOK() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Fed successfully");
  Serial.println("SUCCESS!");
}

void handleFeed() {
  sendOK();
  feeder.write(90);
  delay(1500);
  feeder.write(0);
}

