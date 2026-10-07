/*
  ESP32-S3 Circuit Tester - No TFT
  --------------------------------
  Functions:
    1. DC voltage / battery voltage measurement: 0-12 V recommended
    2. Continuity test (DUT MUST be unpowered)
    3. Serial Monitor status
    4. Local Wi-Fi web dashboard

  Hardware:
    ESP32-S3 Dev Module
    Voltage input: TEST_V+ -> 22k -> ADC GPIO4 -> 4.7k -> GND
    100nF capacitor from GPIO4/ADC node to GND is recommended.
    Continuity: GPIO5 -> 1k resistor -> CONT+ ; CONT- -> GND
    Sense: GPIO6 connected to CONT+ through 10k resistor (optional status sense).

  IMPORTANT:
    - Voltage input is designed for LOW-VOLTAGE DC testing only.
    - Recommended maximum DUT voltage: 12 V DC.
    - NEVER connect mains/AC to the tester.
    - NEVER connect a powered battery/circuit to the continuity port.
    - Reverse polarity protection and an input fuse/polyfuse are recommended.
*/

#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID = "ESP32S3-Circuit-Tester";
const char* AP_PASSWORD = "change-me-123"; // Change before real deployment.

WebServer server(80);

// ADC pin for protected voltage input.
// Divider: 22k (top) + 4.7k (bottom)
constexpr uint8_t VOLTAGE_ADC_PIN = 4;

// Continuity test output.
// DUT is connected between CONT+ and GND.
constexpr uint8_t CONTINUITY_DRIVE_PIN = 5;
constexpr uint8_t CONTINUITY_SENSE_PIN = 6;

constexpr float R_TOP = 22000.0f;
constexpr float R_BOTTOM = 4700.0f;
constexpr float DIVIDER_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM;

constexpr float VOLTAGE_MAX_RECOMMENDED = 12.0f;
constexpr float CONTINUITY_THRESHOLD_OHMS_APPROX = 60.0f;

float readVoltage() {
  // analogReadMilliVolts() uses the ESP32 Arduino ADC calibration path.
  uint32_t mv = analogReadMilliVolts(VOLTAGE_ADC_PIN);
  float adcVoltage = mv / 1000.0f;
  float inputVoltage = adcVoltage * DIVIDER_RATIO;

  if (inputVoltage < 0.08f) inputVoltage = 0.0f;
  return inputVoltage;
}

bool continuityTest() {
  // This pin must only drive the isolated, unpowered continuity DUT.
  pinMode(CONTINUITY_DRIVE_PIN, OUTPUT);
  digitalWrite(CONTINUITY_DRIVE_PIN, HIGH);

  delay(3);
  int sense = digitalRead(CONTINUITY_SENSE_PIN);

  digitalWrite(CONTINUITY_DRIVE_PIN, LOW);
  pinMode(CONTINUITY_DRIVE_PIN, INPUT);

  return sense == LOW;
}

String voltageState(float v) {
  if (v < 0.08f) return "NO INPUT";
  if (v <= 1.1f) return "VERY LOW";
  if (v <= 1.6f) return "1.5V CLASS";
  if (v <= 4.3f) return "LOW VOLTAGE";
  if (v <= 5.5f) return "5V CLASS";
  if (v <= 8.0f) return "MID VOLTAGE";
  if (v <= 13.0f) return "12V CLASS / HIGH";
  return "OVER RANGE";
}

String jsonEscape(const String& s) {
  String out = s;
  out.replace("\"", "\\\"");
  return out;
}

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#111827">
<title>ESP32-S3 Circuit Tester</title>
<style>
:root{color-scheme:dark;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
body{margin:0;background:#0b1220;color:#e5e7eb}
main{max-width:720px;margin:auto;padding:20px}
.card{background:#111827;border:1px solid #263244;border-radius:16px;padding:18px;margin:12px 0}
h1{font-size:24px;margin:0 0 6px}
h2{font-size:18px;margin:0 0 12px}
.value{font-size:42px;font-weight:800;margin:8px 0}
.badge{display:inline-block;padding:6px 10px;border-radius:999px;background:#1f2937}
button{width:100%;padding:14px;margin-top:8px;border:0;border-radius:12px;background:#2563eb;color:white;font-size:16px;font-weight:700}
button.secondary{background:#374151}
.warn{line-height:1.5;background:#2a1d08;border-color:#6b4b12}
.small{font-size:13px;color:#9ca3af;line-height:1.5}
.row{display:flex;gap:10px}.row>*{flex:1}
</style>
</head>
<body>
<main>
  <div class="card">
    <h1>ESP32-S3 Circuit Tester</h1>
    <div class="small">No TFT • browser + Serial Monitor</div>
  </div>

  <div class="card">
    <h2>DC Voltage / Battery</h2>
    <div class="value"><span id="v">--</span> V</div>
    <div class="badge" id="vs">Reading...</div>
    <button onclick="readVoltage()">Measure Voltage</button>
  </div>

  <div class="card">
    <h2>Continuity</h2>
    <div class="value" id="c">--</div>
    <div class="small">Connect only an UNPOWERED wire/component between CONT+ and GND.</div>
    <button onclick="testContinuity()">Test Continuity</button>
  </div>

  <div class="card warn">
    <strong>Safety</strong>
    <div class="small">
      Voltage port: low-voltage DC only, recommended maximum 12 V.
      Never test mains/AC. Never connect a powered battery to the continuity port.
      Use the external protection circuit described in README.md.
    </div>
  </div>
</main>
<script>
async function readVoltage(){
  const r=await fetch('/api/voltage'); const d=await r.json();
  document.getElementById('v').textContent=d.voltage.toFixed(2);
  document.getElementById('vs').textContent=d.state;
}
async function testContinuity(){
  const r=await fetch('/api/continuity'); const d=await r.json();
  document.getElementById('c').textContent=d.continuity?'CONTINUITY':'OPEN';
}
readVoltage();
</script>
</body>
</html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleVoltage() {
  float v = readVoltage();

  String json = "{";
  json += "\"voltage\":";
  json += String(v, 3);
  json += ",\"state\":\"";
  json += jsonEscape(voltageState(v));
  json += "\"";
  json += ",\"max_recommended\":";
  json += String(VOLTAGE_MAX_RECOMMENDED, 1);
  json += "}";

  server.send(200, "application/json", json);

  Serial.print("[VOLTAGE] ");
  Serial.print(v, 3);
  Serial.print(" V | ");
  Serial.println(voltageState(v));
}

void handleContinuity() {
  bool connected = continuityTest();

  String json = "{\"continuity\":";
  json += connected ? "true" : "false";
  json += "}";

  server.send(200, "application/json", json);

  Serial.print("[CONTINUITY] ");
  Serial.println(connected ? "CONTINUITY" : "OPEN");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  analogSetPinAttenuation(VOLTAGE_ADC_PIN, ADC_11db);

  pinMode(CONTINUITY_DRIVE_PIN, INPUT);
  pinMode(CONTINUITY_SENSE_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_AP);
  bool apOK = WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/voltage", HTTP_GET, handleVoltage);
  server.on("/api/continuity", HTTP_GET, handleContinuity);
  server.begin();

  Serial.println();
  Serial.println("================================");
  Serial.println(" ESP32-S3 CIRCUIT TESTER");
  Serial.println("================================");
  Serial.print("AP started: ");
  Serial.println(apOK ? "YES" : "NO");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("Web UI: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println();
  Serial.println("Voltage: TEST_V+ -> divider -> GPIO4, TEST_V- -> GND");
  Serial.println("Continuity: CONT+ -> 1k -> GPIO5, CONT- -> GND");
  Serial.println("WARNING: continuity port must be UNPOWERED.");
}

void loop() {
  server.handleClient();
}
