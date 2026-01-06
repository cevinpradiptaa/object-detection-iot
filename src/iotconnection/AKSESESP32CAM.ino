#include <WiFi.h>
#include <WebServer.h>
#include <esp32cam.h>

#define FLASH_GPIO_NUM 4

const char* ssid = "OPPO Reno";
const char* password = "12345678";

WebServer server(80);

static auto resolution = esp32cam::Resolution::find(640, 480);

bool flashState = false;

void updateFlash() {
  digitalWrite(FLASH_GPIO_NUM, flashState ? HIGH : LOW);
}

void handleCapture() {
  updateFlash();
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    server.send(503, "text/plain", "Capture failed");
    return;
  }

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleStream() {
  WiFiClient client = server.client();

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      continue;
    }

    updateFlash();

    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n\r\n");
    frame->writeTo(client);
    server.sendContent("\r\n");

    delay(50);
  }
}

// Endpoint untuk kontrol flash lewat HTTP
void handleFlash() {
  if (server.arg("state") == "on") {
    flashState = true;
  } else {
    flashState = false;
  }
  updateFlash();
  server.send(200, "text/plain", String("Flash ") + (flashState ? "ON" : "OFF"));
}

void setup() {
  Serial.begin(115200);
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  updateFlash();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(resolution);
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  bool ok = Camera.begin(cfg);
  if (!ok) {
    Serial.println("Camera failed");
    while (true) delay(100);
  }

  server.on("/capture", handleCapture);
  server.on("/stream", handleStream);
  server.on("/flash", handleFlash);

  server.begin();
}

void loop() {
  server.handleClient();
}
