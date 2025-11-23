#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// --- Configuration: Change these values ---
const char* ssid = "Richywifi_2.4G_hz";       // **Replace with your Wi-Fi name**
const char* password = "ld0804013242"; // **Replace with your Wi-Fi password**
const int servoPin = 13;                      // The GPIO pin the servo signal is connected to
// ---------------------
bool movingUp = false;

const int inputPin1 = 2; // Lock
const int inputPin2 = 4; // Stopper top
const int inputPin3 = 5; // Stopper bottom
const int outputPin14 = 14; //Go down
const int outputPin15 = 15; //Go up

int previousStatePin1 = LOW; // Tracks state for Pin 1 edge detection
// (previousStatePin3 is NOT needed for level reset, but we need a latch state)
bool latchActive = false;

WebServer server(80);
Servo myServo; // Create a Servo object

//void checkLatchingLogic() {
//    int currentStatePin1 = digitalRead(inputPin1);
//    int currentStatePin3 = digitalRead(inputPin3);
//   
//    if (currentStatePin1 == HIGH && previousStatePin1 == LOW) {
//        if (!latchActive) { // Only set if not already active
//            latchActive = true;
//            digitalWrite(outputPin14, HIGH); // **ACTION: Set Output HIGH**
//            Serial.println("LATCH SET: Pin 1 went HIGH. Output 14 is now HIGH.");
//        }
//    }
//
//    if (latchActive && currentStatePin3 == HIGH) {
//        latchActive = false;
//        digitalWrite(outputPin14, LOW); // **ACTION: Reset Output LOW**
//        Serial.println("LATCH RESET: Pin 3 is HIGH. Output 14 is now LOW.");
//    }
//    
//    // 3. --- Update Previous State for Next Cycle ---
//    previousStatePin1 = currentStatePin1;
//    // previousStatePin3 is NOT updated/used since we are looking for a HIGH level, not an edge.
//}

// --- Motor Control Logic ---
void checkMotorControl() {
    int currentStatePin2 = digitalRead(inputPin2); // Stopper top
    int currentStatePin3 = digitalRead(inputPin3); // Stopper bottom/Start Up
    
    // --- PHASE 1: START UP (Triggered by Pin 3 HIGH) ---
    if (currentStatePin3 == HIGH) {
        // 1. Reset the Down Latch (outputPin14 is handled below)
        latchActive = false; 
        
        // 2. Start UP movement
        digitalWrite(outputPin14, LOW);  // Ensure Go Down is OFF
        digitalWrite(outputPin15, HIGH); // **ACTION: Go Up is ON**
        movingUp = true;
        Serial.println("MOTOR CONTROL: Pin 3 HIGH. STARTING UP.");
    } 
    
    // --- PHASE 2: STOP UP (Triggered by movingUp and Pin 2 HIGH) ---
    // If we are currently moving UP AND hit the Stopper Top (Pin 2 HIGH)
    else if (movingUp && currentStatePin2 == HIGH) {
        digitalWrite(outputPin15, LOW); // **ACTION: Go Up is OFF**
        
        // Prepare for next Latch: set previous state of pin 1 to be HIGH
        previousStatePin1 = HIGH; 
        
        movingUp = false;
        Serial.println("MOTOR CONTROL: Pin 2 HIGH reached. STOPPING UP. Ready for next DOWN command.");
    }
    
    // --- PHASE 3: GO DOWN (Triggered by Latch Active and NOT moving UP) ---
    // This is the action for the Pin 1 trigger (only if we're not moving UP)
    else if (latchActive && !movingUp) {
        digitalWrite(outputPin14, HIGH);
        digitalWrite(outputPin15, LOW); // Ensure Go Up is OFF
    } 
    
    // --- PHASE 4: IDLE/SAFETY ---
    // Ensure all outputs are LOW if no action is commanded
    else if (!latchActive && !movingUp) {
        digitalWrite(outputPin14, LOW);
        digitalWrite(outputPin15, LOW);
    }
}

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

void handleStatus() {
  // Read the current state of the three digital input pins
  int state1 = digitalRead(inputPin1);
  int state2 = digitalRead(inputPin2);//Stopper top
  int state3 = digitalRead(inputPin3);//Stopper Bottom

  int state14 = digitalRead(outputPin14);

  String statusMessage = "--- Device Status ---\n";
    
  server.send(200, "text/plain", statusMessage);
  Serial.println("Status command received. Pins reported.");
}

void setup() {
  Serial.begin(115200);

  pinMode(inputPin1, INPUT_PULLDOWN); 
  pinMode(inputPin2, INPUT_PULLDOWN);
  pinMode(inputPin3, INPUT_PULLDOWN);

  pinMode(outputPin14, OUTPUT);
  digitalWrite(outputPin14, LOW);
  pinMode(outputPin15, OUTPUT);
  digitalWrite(outputPin15, LOW);
  
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
  server.on("/status", handleStatus);
 
  // 4. Start Server
  server.begin();
}

void loop() {
  server.handleClient(); 
  checkMotorControl();
}
