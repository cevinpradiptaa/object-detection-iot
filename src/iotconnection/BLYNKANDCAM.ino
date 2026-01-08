#define BLYNK_TEMPLATE_ID "TMPL6kXUPhEmi"
#define BLYNK_TEMPLATE_NAME "Aplikasi Produktivitas Ayam"
#define BLYNK_AUTH_TOKEN "1OwNej3zHj0zCn1eje-H_ZuGH0UKe8OM"

#include <WiFi.h>
#include <WebServer.h>
#include <esp32cam.h>
#include <BlynkSimpleEsp32.h>
// #include <ESPmDNS.h>  // ← Tambahkan ini

// === KONFIGURASI WIFI DAN BLYNK ===
const char* WIFI_SSID = "OPPO Reno";
const char* WIFI_PASS = "12345678";
char auth[] = BLYNK_AUTH_TOKEN;

// === IP KOMPUTER UNTUK PENGIRIMAN TRIGGER ===
const char* host = "192.168.95.79";  // IP komputer Python
const uint16_t port = 5000;          // Port server Python

// === PENGATURAN CAMERA DAN FLASH ===
#define FLASH_GPIO_NUM 4
WebServer server(80);
bool flashControl = true;

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

// === Fungsi membatasi akses berdasarkan IP ===
// bool isAllowedClient() {
//   IPAddress allowedIP;
//   allowedIP.fromString(host);
//   return server.client().remoteIP() == allowedIP;
// }

void keepFlashOn() {
  digitalWrite(FLASH_GPIO_NUM, flashControl ? HIGH : LOW);
}

void serveJpg() {
  keepFlashOn();
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }

  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  frame->writeTo(server.client());
  keepFlashOn();
}

void handleJpgLo() {
  // if (!isAllowedClient()) {
  //   server.send(403, "text/plain", "Access denied");
  //   Serial.println("Denied: " + server.client().remoteIP().toString());
  //   return;
  // }
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgMid() {
  // if (!isAllowedClient()) {
  //   server.send(403, "text/plain", "Access denied");
  //   Serial.println("Denied: " + server.client().remoteIP().toString());
  //   return;
  // }
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi() {
  // if (!isAllowedClient()) {
  //   server.send(403, "text/plain", "Access denied");
  //   Serial.println("Denied: " + server.client().remoteIP().toString());
  //   return;
  // }
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleStream() {
  // if (!isAllowedClient()) {
  //   server.send(403, "text/plain", "Access denied");
  //   Serial.println("Denied: " + server.client().remoteIP().toString());
  //   return;
  // }

  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      Serial.println("Capture failed");
      continue;
    }

    keepFlashOn();
    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n\r\n");
    frame->writeTo(client);
    server.sendContent("\r\n");
    delay(42);
  }
}

BLYNK_WRITE(V2) {
  int tombol = param.asInt();
  WiFiClient client;
  if (client.connect(host, port)) {
    client.print(tombol == 1 ? "TRIGGER" : "DONE");
    client.stop();
    Serial.println(tombol == 1 ? "TRIGGER sent" : "DONE sent");
  } else {
    Serial.println("Koneksi TCP gagal");
  }
}

BLYNK_WRITE(V3) {
  int state = param.asInt();
  flashControl = (state == 1);
  digitalWrite(FLASH_GPIO_NUM, flashControl ? HIGH : LOW);
  Serial.println(flashControl ? "FLASH ON via Blynk" : "FLASH OFF via Blynk");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, HIGH);

  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(midRes);
  cfg.setBufferCount(2);
  cfg.setJpeg(80);
  bool ok = Camera.begin(cfg);
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // // === Setup mDNS ===
  // if (MDNS.begin("kameraesp32")) {
  //   Serial.println("mDNS responder started");
  //   Serial.println("Access via: http://kameraesp32.local/");
  // } else {
  //   Serial.println("mDNS setup failed");
  // }

  Blynk.begin(auth, WIFI_SSID, WIFI_PASS);

  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-mid.jpg", handleJpgMid);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/stream", handleStream);
  server.begin();
}

void loop() {
  // keepFlashOn();
  server.handleClient();
  keepFlashOn();
  Blynk.run();
}
