/*
 * Smart Mine Safety Rover - TX Controller
 * ---------------------------------------
 * GitHub-ready TX-side dashboard firmware scaffold.
 *
 * IMPORTANT:
 * This file is intentionally kept hardware-neutral for the radio/GPS pin
 * mapping because the exact final pin map from the working prototype was
 * not available as a readable source file in this chat.
 *
 * Dashboard:
 *   Wi-Fi AP : SMART_MINE_ROVER
 *   Password : mine12345
 *   URL      : http://192.168.4.1
 *
 * The browser dashboard is served directly by the ESP32.
 *
 * For the exact hardware build, merge this dashboard section into the
 * already-working main.cpp that contains your final LoRa/OLED/GPS/button
 * pin definitions and packet format.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* AP_SSID = "SMART_MINE_ROVER";
const char* AP_PASSWORD = "mine12345";

String gasPpm = "--";
String mq6Voltage = "--";
String gasAlert = "NO";
String gpsLat = "--";
String gpsLon = "--";
String gpsStatus = "WAITING FOR FIX...";
String lastPacket = "No telemetry received";
String chState[4] = {"OFF", "OFF", "OFF", "OFF"};

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SMART MINE ROVER</title>
<style>
:root{
  --bg:#07111c;--panel:#0d1b2a;--line:#1e3448;--text:#e8f1f8;
  --muted:#8da4b7;--accent:#31d6ff;--good:#39e58c;--warn:#ffbf4a;
  --bad:#ff5c67;
}
*{box-sizing:border-box}
body{margin:0;background:radial-gradient(circle at top,#10283a 0,#07111c 48%,#040a10 100%);
font-family:Inter,Segoe UI,Arial,sans-serif;color:var(--text)}
.wrap{max-width:1400px;margin:auto;padding:20px}
header{display:flex;justify-content:space-between;gap:20px;align-items:center;margin-bottom:18px}
.brand{font-size:26px;font-weight:800;letter-spacing:.08em}
.sub{color:var(--muted);font-size:13px;margin-top:4px}
.badge{border:1px solid #28506a;background:#0a2030;padding:9px 13px;border-radius:999px;color:var(--accent);font-size:12px}
.grid{display:grid;grid-template-columns:1.5fr 1fr;gap:16px}
.card{background:linear-gradient(180deg,#0d1b2a,#0a1724);border:1px solid var(--line);
border-radius:18px;padding:17px;box-shadow:0 12px 35px #0005}
.card h2{font-size:14px;letter-spacing:.12em;margin:0 0 14px;color:#b9cad7}
.map{height:420px;border-radius:14px;border:1px solid #29475c;
background:
linear-gradient(30deg,#0d2532 12%,transparent 12.5%,transparent 87%,#0d2532 87.5%,#0d2532),
linear-gradient(150deg,#0d2532 12%,transparent 12.5%,transparent 87%,#0d2532 87.5%,#0d2532),
linear-gradient(30deg,#0d2532 12%,transparent 12.5%,transparent 87%,#0d2532 87.5%,#0d2532),
linear-gradient(150deg,#0d2532 12%,transparent 12.5%,transparent 87%,#0d2532 87.5%,#0d2532);
background-size:80px 140px;background-position:0 0,0 0,40px 70px,40px 70px;
position:relative;overflow:hidden}
.cross{position:absolute;left:50%;top:50%;width:28px;height:28px;transform:translate(-50%,-50%)}
.cross:before,.cross:after{content:"";position:absolute;background:var(--accent);box-shadow:0 0 18px var(--accent)}
.cross:before{width:28px;height:2px;top:13px}.cross:after{height:28px;width:2px;left:13px}
.coords{position:absolute;left:14px;bottom:14px;background:#06111add;border:1px solid #2b5065;
padding:10px 12px;border-radius:10px;font-family:monospace;font-size:13px}
.cards{display:grid;grid-template-columns:repeat(2,1fr);gap:12px}
.metric{padding:14px;border:1px solid var(--line);border-radius:14px;background:#081522}
.label{font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.1em}
.value{font-size:27px;font-weight:750;margin-top:5px}
.unit{font-size:12px;color:var(--muted)}
.ok{color:var(--good)}.warn{color:var(--warn)}.bad{color:var(--bad)}
.telemetry{font-family:ui-monospace,Consolas,monospace;color:#9fdcff;font-size:12px;
white-space:pre-wrap;word-break:break-word;background:#06101a;border-radius:10px;padding:12px}
footer{margin-top:15px;color:#6f8799;font-size:11px;text-align:center}
@media(max-width:900px){.grid{grid-template-columns:1fr}.map{height:330px}}
</style>
</head>
<body>
<div class="wrap">
<header>
  <div>
    <div class="brand">SMART MINE ROVER</div>
    <div class="sub">Underground safety telemetry • TX command & monitoring node</div>
  </div>
  <div class="badge">● ESP32 ONLINE</div>
</header>

<div class="grid">
  <section class="card">
    <h2>LIVE LOCATION</h2>
    <div class="map">
      <div class="cross"></div>
      <div class="coords">
        GPS <b id="gpsStatus">WAITING FOR FIX...</b><br>
        LAT <span id="lat">--</span><br>
        LON <span id="lon">--</span>
      </div>
    </div>
  </section>

  <section>
    <div class="card">
      <h2>SAFETY TELEMETRY</h2>
      <div class="cards">
        <div class="metric"><div class="label">Gas concentration</div><div class="value" id="gas">--</div><div class="unit">PPM</div></div>
        <div class="metric"><div class="label">MQ-6 voltage</div><div class="value" id="volt">--</div><div class="unit">VOLTS</div></div>
        <div class="metric"><div class="label">Gas alert</div><div class="value ok" id="alert">NO</div><div class="unit">STATUS</div></div>
        <div class="metric"><div class="label">GPS</div><div class="value" id="gps">WAIT</div><div class="unit">FIX STATUS</div></div>
      </div>
    </div>

    <div class="card" style="margin-top:16px">
      <h2>RADIO / PACKET STATUS</h2>
      <div class="telemetry" id="packet">No telemetry received</div>
    </div>
  </section>
</div>

<footer>SMART MINE ROVER • Local ESP32 dashboard • No internet connection required</footer>
</div>

<script>
async function update(){
  try{
    const r=await fetch('/api/status',{cache:'no-store'});
    const d=await r.json();
    document.getElementById('gas').textContent=d.gas;
    document.getElementById('volt').textContent=d.voltage;
    document.getElementById('alert').textContent=d.alert;
    document.getElementById('alert').className='value '+(d.alert==='YES'?'bad':'ok');
    document.getElementById('gps').textContent=d.gpsStatus==='FIXED'?'FIXED':'WAIT';
    document.getElementById('gpsStatus').textContent=d.gpsStatus;
    document.getElementById('lat').textContent=d.lat;
    document.getElementById('lon').textContent=d.lon;
    document.getElementById('packet').textContent=d.packet;
  }catch(e){}
}
setInterval(update,1000); update();
</script>
</body>
</html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleStatus() {
  String json = "{";
  json += "\"gas\":\"" + gasPpm + "\",";
  json += "\"voltage\":\"" + mq6Voltage + "\",";
  json += "\"alert\":\"" + gasAlert + "\",";
  json += "\"gpsStatus\":\"" + gpsStatus + "\",";
  json += "\"lat\":\"" + gpsLat + "\",";
  json += "\"lon\":\"" + gpsLon + "\",";
  json += "\"packet\":\"" + lastPacket + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

/*
 * Expected telemetry format from the RX/rover side:
 *
 * GAS:24,V:0.98,ALERT:0
 *
 * If your existing LoRa receiver already produces this packet, call
 * processTelemetry(packet) from that existing LoRa receive section.
 */
void processTelemetry(const String& packet) {
  lastPacket = packet;

  int p = packet.indexOf("GAS:");
  if (p >= 0) {
    int e = packet.indexOf(',', p);
    gasPpm = packet.substring(p + 4, e < 0 ? packet.length() : e);
  }

  p = packet.indexOf("V:");
  if (p >= 0) {
    int e = packet.indexOf(',', p);
    mq6Voltage = packet.substring(p + 2, e < 0 ? packet.length() : e);
  }

  p = packet.indexOf("ALERT:");
  if (p >= 0) {
    String a = packet.substring(p + 6);
    int e = a.indexOf(',');
    if (e >= 0) a = a.substring(0, e);
    gasAlert = (a == "1") ? "YES" : "NO";
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.begin();

  Serial.println();
  Serial.println("================================");
  Serial.println("       WIFI DASHBOARD");
  Serial.println("================================");
  Serial.println("Wi-Fi Access Point started!");
  Serial.print("Wi-Fi SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Wi-Fi Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("Dashboard IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();

  /*
   * Keep your existing LoRa/GPS/OLED/button code here.
   *
   * Example:
   * if (LoRa.parsePacket()) {
   *   String packet = "";
   *   while (LoRa.available()) packet += (char)LoRa.read();
   *   processTelemetry(packet);
   * }
   */
}
