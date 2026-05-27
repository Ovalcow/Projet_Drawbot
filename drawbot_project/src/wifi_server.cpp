#include "wifi_server.h"

#include <WebServer.h>
#include <WiFi.h>

#include "config.h"
#include "encoders.h"
#include "motors.h"
#include "sensors.h"
#include "sequences.h"

namespace {
WebServer server(80);
SpeedProfile currentProfile = SpeedProfile::Normal;

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Drawbot</title>
  <style>
    :root { color-scheme: light dark; font-family: Arial, sans-serif; }
    body { margin: 0; background: #f5f7fb; color: #182033; }
    main { width: min(980px, calc(100% - 24px)); margin: 0 auto; padding: 18px 0 32px; }
    h1 { margin: 0 0 12px; font-size: 2rem; }
    section { background: white; border: 1px solid #d8deea; border-radius: 8px; padding: 14px; margin: 12px 0; }
    h2 { margin: 0 0 12px; font-size: 1.1rem; }
    button, select, input { font: inherit; }
    button { border: 0; border-radius: 7px; padding: 12px 14px; background: #2056d6; color: white; cursor: pointer; min-height: 44px; }
    button:hover { filter: brightness(1.08); }
    button.stop { background: #d61f34; font-weight: 700; width: 100%; font-size: 1.1rem; }
    button.secondary { background: #43506a; }
    button.diag { background: #087b65; }
    .grid { display: grid; gap: 10px; }
    .pad { grid-template-columns: repeat(3, minmax(70px, 1fr)); max-width: 420px; }
    .pad .wide { grid-column: 1 / span 3; }
    .row { display: flex; gap: 10px; flex-wrap: wrap; align-items: center; }
    input, select { min-height: 42px; border: 1px solid #b7c0d4; border-radius: 7px; padding: 0 10px; background: white; color: #182033; }
    pre { white-space: pre-wrap; background: #111827; color: #d7f9e9; border-radius: 7px; padding: 12px; min-height: 84px; }
    @media (prefers-color-scheme: dark) {
      body { background: #111827; color: #edf2ff; }
      section { background: #1b2436; border-color: #34405a; }
      input, select { background: #111827; color: #edf2ff; border-color: #4d5b78; }
    }
  </style>
</head>
<body>
<main>
  <h1>Drawbot</h1>
  <section>
    <button class="stop" onclick="cmd('STOP')">STOP URGENCE</button>
  </section>
  <section>
    <h2>Controle manuel</h2>
    <div class="grid pad">
      <span></span><button onclick="move('FWD')">Avancer</button><span></span>
      <button onclick="move('TL')">Gauche</button><button class="secondary" onclick="cmd('STOP')">Stop</button><button onclick="move('TR')">Droite</button>
      <span></span><button onclick="move('BCK')">Reculer</button><span></span>
    </div>
    <p class="row">
      <label>Profil
        <select id="profile" onchange="cmd('PROFILE,' + this.value)">
          <option value="0">Lent</option>
          <option value="1" selected>Normal</option>
          <option value="2">Rapide</option>
        </select>
      </label>
      <label>PWM <input id="pwm" type="number" min="0" max="255" value="170"></label>
      <label>Duree ms <input id="ms" type="number" min="50" max="15000" value="350"></label>
    </p>
  </section>
  <section>
    <h2>Sequences</h2>
    <div class="row">
      <button onclick="cmd('S1')">Escalier</button>
      <button onclick="circle()">Cercle</button>
      <button onclick="cmd('S3')">Fleche / rose</button>
      <label>Rayon cm <input id="radius" type="number" min="2" max="20" step="0.5" value="10"></label>
    </div>
  </section>
  <section>
    <h2>Diagnostic</h2>
    <div class="row">
      <button class="diag" onclick="cmd('TEST_MOTORS')">TEST_MOTORS</button>
      <button class="diag" onclick="cmd('TEST_PWM')">TEST_PWM</button>
      <button class="diag" onclick="cmd('TEST_DIR')">TEST_DIR</button>
    </div>
  </section>
  <section>
    <h2>Statut</h2>
    <pre id="status">Chargement...</pre>
  </section>
</main>
<script>
async function cmd(c) {
  const r = await fetch('/cmd?c=' + encodeURIComponent(c));
  document.getElementById('status').textContent = await r.text();
  refresh();
}
function move(kind) {
  const pwm = document.getElementById('pwm').value || 170;
  const ms = document.getElementById('ms').value || 350;
  cmd(kind + ',' + pwm + ',' + ms);
}
function circle() {
  const radius = document.getElementById('radius').value || 10;
  cmd('S2,' + radius);
}
async function refresh() {
  const r = await fetch('/cmd?c=STATUS');
  document.getElementById('status').textContent = await r.text();
}
setInterval(refresh, 1000);
refresh();
</script>
</body>
</html>
)HTML";

const ProfileSettings& profileSettings() {
  return SPEED_PROFILES[static_cast<uint8_t>(currentProfile)];
}

String getToken(const String& text, uint8_t index) {
  uint8_t current = 0;
  int start = 0;
  while (true) {
    const int comma = text.indexOf(',', start);
    if (current == index) {
      return comma < 0 ? text.substring(start) : text.substring(start, comma);
    }
    if (comma < 0) {
      return "";
    }
    start = comma + 1;
    current++;
  }
}

uint32_t parseDuration(const String& command, uint8_t index) {
  const String token = getToken(command, index);
  if (token.length() == 0) {
    return DEFAULT_MANUAL_DURATION_MS;
  }
  const long rawDuration = token.toInt();
  return clampDuration(static_cast<uint32_t>(rawDuration < 0 ? 0 : rawDuration));
}

int parsePwm(const String& command, uint8_t index, uint8_t fallback) {
  const String token = getToken(command, index);
  if (token.length() == 0) {
    return fallback;
  }
  return clampPwm(token.toInt());
}

void sendText(int code, const String& text) {
  server.send(code, "text/plain; charset=utf-8", text);
}

String ok(const String& message) {
  return "OK: " + message + "\n" + buildStatusJson();
}

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleCommand() {
  if (!server.hasArg("c")) {
    sendText(400, "ERREUR: parametre c manquant");
    return;
  }

  String command = server.arg("c");
  command.trim();
  command.toUpperCase();


  Serial.printf("[HTTP] Commande recue: %s\n", command.c_str());

  const String verb = getToken(command, 0);

  if (verb == "STOP") {
    stopSequences();
    sendText(200, ok("STOP"));
    return;
  }

  if (verb == "STATUS") {
    sendText(200, buildStatusJson());
    return;
  }

  if (verb == "PROFILE") {
    const int profile = getToken(command, 1).toInt();
    if (profile < 0 || profile >= SPEED_PROFILE_COUNT) {
      sendText(400, "ERREUR: profil invalide, attendu 0/1/2");
      return;
    }
    currentProfile = static_cast<SpeedProfile>(profile);
    Serial.printf("[HTTP] Profil vitesse=%s\n", profileSettings().name);
    sendText(200, ok(String("Profil ") + profileSettings().name));
    return;
  }

  const ProfileSettings& profile = profileSettings();

  if (verb == "FWD" || verb == "BCK") {
    stopSequences();
    const int pwm = parsePwm(command, 1, profile.straightPwm);
    const uint32_t duration = parseDuration(command, 2);
    Serial.printf("[HTTP] Mouvement %s PWM=%d duree=%lu\n", verb.c_str(), pwm,
                  static_cast<unsigned long>(duration));
    moveRobotStraight(pwm, duration, verb == "FWD");
    sendText(200, ok(verb));
    return;
  }

  if (verb == "TL" || verb == "TR") {
    stopSequences();
    const int pwm = parsePwm(command, 1, profile.turnPwm);
    const uint32_t duration = parseDuration(command, 2);
    Serial.printf("[HTTP] Virage %s PWM=%d duree=%lu\n", verb.c_str(), pwm,
                  static_cast<unsigned long>(duration));
    turnRobot(verb == "TL", pwm, duration);
    sendText(200, ok(verb));
    return;
  }

  if (verb == "S1" || verb == "ESCALIER") {
    sendText(startSequenceEscalier() ? 200 : 500, ok("Sequence escalier"));
    return;
  }

  if (verb == "S2" || verb == "CERCLE") {
    const float radius = getToken(command, 1).length() ? getToken(command, 1).toFloat() : 10.0f;
    sendText(startSequenceCircle(radius) ? 200 : 500, ok("Sequence cercle"));
    return;
  }

  if (verb == "S3" || verb == "ROSE") {
    sendText(startSequenceRose() ? 200 : 500, ok("Sequence rose/fleche"));
    return;
  }

  if (verb == "TEST_MOTORS") {
    sendText(startDiagnosticMotors() ? 200 : 500, ok("Diagnostic moteurs"));
    return;
  }

  if (verb == "TEST_PWM") {
    sendText(startDiagnosticPWM() ? 200 : 500, ok("Diagnostic PWM"));
    return;
  }

  if (verb == "TEST_DIR") {
    sendText(startDiagnosticDirections() ? 200 : 500, ok("Diagnostic directions"));
    return;
  }

  sendText(400, "ERREUR: commande inconnue: " + verb);
}
}  // namespace

void setupWifiServer() {
  WiFi.mode(WIFI_AP);
  const bool apStarted = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  if (!apStarted) {
    Serial.println("[WIFI][ERREUR] Impossible de demarrer le point d'acces");
  }

  Serial.printf("[WIFI] Point d'acces actif SSID=%s\n", WIFI_AP_SSID);
  Serial.printf("[WIFI] IP: %s\n", WiFi.softAPIP().toString().c_str());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCommand);
  server.onNotFound([]() {
    sendText(404, "ERREUR: route inconnue");
  });
  server.begin();
  Serial.println("[WIFI] Serveur HTTP pret sur http://192.168.4.1");
}

void handleWifiServer() {
  server.handleClient();
}

String buildStatusJson() {
  const SensorStatus sensors = getSensorStatus();
  String json = "{\n";
  json += "  \"wifi\":\"";
  json += WIFI_AP_SSID;
  json += "\",\n  \"ip\":\"";
  json += WiFi.softAPIP().toString();
  json += "\",\n  \"profile\":\"";
  json += profileSettings().name;
  json += "\",\n  \"sequence\":\"";
  json += sequenceStatusText();
  json += "\",\n  \"motion\":\"";
  json += motionKindToString(currentMotionKind());
  json += "\",\n  \"leftPower\":";
  json += String(currentLeftPower());
  json += ",\n  \"rightPower\":";
  json += String(currentRightPower());
  json += ",\n  \"remainingMs\":";
  json += String(currentMotionRemainingMs());
  json += ",\n  \"encLeft\":";
  json += String(getLeftEncoderTicks());
  json += ",\n  \"encRight\":";
  json += String(getRightEncoderTicks());
  json += ",\n  \"imu\":";
  json += sensors.imuDetected ? "true" : "false";
  json += ",\n  \"mag\":";
  json += sensors.magnetometerDetected ? "true" : "false";
  json += "\n}";
  return json;
}
