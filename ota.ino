#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Update.h>

const char* ssid = "The 7040 Garage";
const char* password = "Harshu@7040";

WebServer server(80);

Servo myServo;

#define SERVO_PIN 13
#define LED_PIN 2
#define BUZZER_PIN 4

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);

  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(1000);
    Serial.println("...");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {

    String page = "<h1>ESP32 OTA System</h1>";
    page += "<a href='/update'>Update Firmware</a>";

    server.send(200, "text/html", page);
  });

  server.on("/update", HTTP_GET, []() {

    server.sendHeader("Connection", "close");

    server.send(200, "text/html",
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'>"
      "<input type='submit' value='Upload'>"
      "</form>");
  });

  server.on("/update", HTTP_POST, []() {

    server.sendHeader("Connection", "close");

    server.send(200, "text/plain",
      Update.hasError() ? "Update Failed" : "Update Success. Rebooting...");

    delay(1000);

    ESP.restart();

  }, []() {

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {

      Serial.printf("Update Start: %s\n", upload.filename.c_str());

      digitalWrite(LED_PIN, HIGH);

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }

    } else if (upload.status == UPLOAD_FILE_WRITE) {

      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }

    } else if (upload.status == UPLOAD_FILE_END) {

      if (Update.end(true)) {

        Serial.printf("Update Success: %u bytes\n", upload.totalSize);

        for(int i = 0; i < 3; i++) {

          digitalWrite(BUZZER_PIN, HIGH);
          delay(200);

          digitalWrite(BUZZER_PIN, LOW);
          delay(200);
        }

      } else {

        Update.printError(Serial);
      }

      digitalWrite(LED_PIN, LOW);
    }
  });

  server.begin();

  Serial.println("OTA Server Started");
}

void loop() {

  server.handleClient();

  // LED Blink
  digitalWrite(LED_PIN, HIGH);

  // Servo 0 Degree
  myServo.write(0);

  delay(200);

  digitalWrite(LED_PIN, LOW);

  // Servo 90 Degree
  myServo.write(90);

  delay(1000);

  // Double Beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);

  digitalWrite(BUZZER_PIN, LOW);
  delay(200);

  // Servo 180 Degree
  myServo.write(180);

  delay(1000);
}