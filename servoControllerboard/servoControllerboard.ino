#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// --- Configuration: Change these values ---
const char* ssid = "Richywifi_2.4G_hz";       // **Replace with your Wi-Fi name**
const char* password = "ld0804013242"; // **Replace with your Wi-Fi password**
const int servoPin = 13;                      // The GPIO pin the servo signal is connected to
// ---------------------

WebServer server(80);
Servo myServo; // Create a Servo object

void handleRoot() {
  server.send(200, "text/plain", "ESP32 Servo Controller Ready. Use /0 or /90 paths.");
}

void handleZero() {
  Serial.println("Command received: 0 -> Setting servo to 0 degrees.");
  myServo.write(0); // Set the servo to 0 degrees
  
  // Confirmation sent back to the client/browser
  server.send(200, "text/plain", "ACK: Servo set to 0 degrees.");
}

void handleNinety() {
  Serial.println("Command received: 90 -> Setting servo to 90 degrees.");
  myServo.write(90); // Set the servo to 90 degrees
  
  // Confirmation sent back to the client/browser
  server.send(200, "text/plain", "ACK: Servo set to 90 degrees.");
}

void setup() {
  Serial.begin(115200);
  
  // 1. Servo Setup
  myServo.setPeriodHertz(50);      // Standard 50 Hz servo signal
  myServo.attach(servoPin);        // Attach the servo object to the pin
  myServo.write(0);                // Initialize the servo position to 0 degrees
  
  // 2. Wi-Fi Connection
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); 
  
  // 3. Server Handlers Setup
  server.on("/", handleRoot); 
  server.on("/0", handleZero); 
  server.on("/90", handleNinety); 
  
  // 4. Start Server
  server.begin();
}

void loop() {
  server.handleClient(); 
}
