#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Converge_2.4GHz_RGUSHK";
const char* password = "y9NTQPKN";

String serverURL = "http://192.168.1.13/smart/hardware.php";

const int TRIG_PIN = 23;
const int ECHO_PIN = 22;

int LED_GREEN = 19;
int LED_YELLOW = 18;
int LED_RED = 5;

long duration;
float distanceCm;

int binID = 1;

// 📌 SET YOUR BIN HEIGHT HERE (distance when EMPTY)
float binHeight = 30.0;  // Example: 30 cm deep trash bin

// Keep track of last sent status to avoid duplicate notifications
String lastStatus = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📡 IP: ");
  Serial.println(WiFi.localIP());

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  // Trigger pulse
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) distanceCm = binHeight; 
  else distanceCm = (duration * 0.034) / 2.0;

  // 🔥 CAPACITY FORMULA
  float capacity = (1 - (distanceCm / binHeight)) * 100.0;
  if (capacity < 0) capacity = 0;
  if (capacity > 100) capacity = 100;

  // 📌 Determine STATUS using your rules
  String status = "";
  if (capacity == 0) {
    status = "empty";
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
  }
  else if (capacity >= 10 && capacity <= 50) {
    status = "half_full";
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
  }
  else if (capacity >= 80) {  // FULL: 80%-100%
    status = "full";
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);
  }
  else {
    // Between 51%–79% → treat as half_full
    status = "half_full";
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
  }

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.print(" cm | Capacity: ");
  Serial.print(capacity);
  Serial.print("% | Status: ");
  Serial.println(status);

  // ⏳ Send to server only if status changed
  if (status != lastStatus && WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = serverURL +
      "?bin_id=" + binID +
      "&status=" + status +
      "&capacity=" + String(capacity);

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Server response: " + http.getString());
    } else {
      Serial.println("❌ Failed to send");
    }
    http.end();

    lastStatus = status; // update lastStatus to prevent duplicates
  }

  delay(2000); // measurement interval
}
