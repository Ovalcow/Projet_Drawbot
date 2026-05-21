#include "wifi_server.h"
#include "motors.h"
#include "sequences.h"
#include "config.h"

#include <WebServer.h>
#include <WiFi.h>

RobotProfile activeProfile = PROFILES[DEFAULT_PROFILE_INDEX];

static int activeProfileIdx = DEFAULT_PROFILE_INDEX;
static bool wifiActive = false;
static WebServer server(HTTP_PORT);

static unsigned long lastButtonPress = 0;
static const unsigned long DEBOUNCE_MS = 300;

static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Drawbot</title>
<style>
  :root {
    --bg:#101115; --surface:#1b1d25; --border:#343846; --accent:#4f8cff;
    --accent2:#ff6b6b; --text:#f0f2f5; --muted:#9aa1ad; --green:#4ade80; --red:#f87171;
  }
  * { box-sizing:border-box; margin:0; padding:0; }
  body { background:var(--bg); color:var(--text); font-family:system-ui,sans-serif; min-height:100vh; padding:16px; }
  h1 { text-align:center; font-size:1.5rem; color:var(--accent); margin-bottom:4px; }
  .subtitle { text-align:center; font-size:.8rem; color:var(--muted); margin-bottom:16px; }
  .card { background:var(--surface); border:1px solid var(--border); border-radius:8px; padding:14px; margin-bottom:12px; }
  .card h2 { font-size:.78rem; letter-spacing:1.5px; color:var(--muted); text-transform:uppercase; margin-bottom:12px; }
  #status-bar { display:flex; align-items:center; gap:8px; font-size:.9rem; }
  #status-dot { width:10px; height:10px; border-radius:50%; background:var(--green); flex-shrink:0; }
  #status-txt { flex:1; }
  #profile-badge { background:var(--accent); border-radius:16px; padding:2px 10px; font-size:.75rem; }
  .dpad { display:grid; grid-template-columns:repeat(3,1fr); gap:8px; max-width:220px; margin:0 auto; }
  .dpad-btn { aspect-ratio:1; border:none; border-radius:8px; background:var(--border); color:var(--text); font-size:1.35rem; cursor:pointer; display:flex; align-items:center; justify-content:center; }
  .dpad-btn:active,.dpad-btn.pressed { background:var(--accent); transform:scale(.94); }
  .dpad-btn.stop { background:#38222a; color:var(--red); font-size:1rem; }
  .dpad-btn.empty { background:transparent; pointer-events:none; }
  #joystick-area { display:flex; flex-direction:column; align-items:center; gap:10px; }
  #joystick-wrap { position:relative; width:180px; height:180px; background:var(--border); border-radius:50%; border:2px solid var(--accent); touch-action:none; }
  #joystick-knob { position:absolute; width:60px; height:60px; background:var(--accent); border-radius:50%; top:90px; left:90px; transform:translate(-50%,-50%); pointer-events:none; }
  #joy-values { font-size:.75rem; color:var(--muted); }
  #joy-toggle { padding:8px 18px; border:1px solid var(--accent); border-radius:8px; background:transparent; color:var(--accent); cursor:pointer; }
  #joy-toggle.active { background:var(--accent); color:white; }
  .seq-grid,.profile-grid { display:grid; gap:8px; }
  .seq-grid { grid-template-columns:repeat(2,1fr); }
  .profile-grid { grid-template-columns:repeat(3,1fr); }
  .seq-btn,.profile-btn { padding:11px 6px; border:1px solid var(--border); border-radius:8px; background:transparent; color:var(--text); cursor:pointer; font-size:.82rem; }
  .seq-btn:active { background:var(--accent2); border-color:var(--accent2); }
  .profile-btn { color:var(--muted); }
  .profile-btn.active { border-color:var(--accent); color:var(--text); background:#273144; }
  .manual-row { display:flex; gap:8px; align-items:center; flex-wrap:wrap; }
  .manual-row input,.manual-row select { flex:1; min-width:74px; padding:8px; border-radius:8px; border:1px solid var(--border); background:#10131a; color:var(--text); }
  .send-btn { padding:8px 14px; border:none; border-radius:8px; background:var(--accent); color:white; cursor:pointer; }
  #log { font-size:.72rem; color:var(--muted); font-family:monospace; max-height:90px; overflow-y:auto; }
  #log p { margin:2px 0; }
  #log p.ok { color:var(--green); }
  #log p.err { color:var(--red); }
</style>
</head>
<body>
<h1>DRAWBOT</h1>
<p class="subtitle">Controle WiFi - http://192.168.4.1</p>

<div class="card">
  <div id="status-bar">
    <div id="status-dot"></div>
    <span id="status-txt">Connecte</span>
    <span id="profile-badge">Normal</span>
  </div>
</div>

<div class="card">
  <h2>Deplacement</h2>
  <div class="dpad">
    <div class="dpad-btn empty"></div>
    <button class="dpad-btn" id="btn-fwd" onpointerdown="dpadPress('fwd')" onpointerup="dpadRelease()" onpointerleave="dpadRelease()">AV</button>
    <div class="dpad-btn empty"></div>
    <button class="dpad-btn" id="btn-tl" onpointerdown="dpadPress('tl')" onpointerup="dpadRelease()" onpointerleave="dpadRelease()">G</button>
    <button class="dpad-btn stop" onclick="sendCmd('STOP')">STOP</button>
    <button class="dpad-btn" id="btn-tr" onpointerdown="dpadPress('tr')" onpointerup="dpadRelease()" onpointerleave="dpadRelease()">D</button>
    <div class="dpad-btn empty"></div>
    <button class="dpad-btn" id="btn-bck" onpointerdown="dpadPress('bck')" onpointerup="dpadRelease()" onpointerleave="dpadRelease()">AR</button>
    <div class="dpad-btn empty"></div>
  </div>
</div>

<div class="card">
  <h2>Joystick</h2>
  <div id="joystick-area">
    <div id="joystick-wrap"><div id="joystick-knob"></div></div>
    <div id="joy-values">x: 0 | y: 0</div>
    <button id="joy-toggle" onclick="toggleJoystick()">Activer joystick</button>
  </div>
</div>

<div class="card">
  <h2>Sequences</h2>
  <div class="seq-grid">
    <button class="seq-btn" onclick="sendCmd('S1')">S1<br>Escalier</button>
    <button class="seq-btn" onclick="sendCmd('S2,20')">S2<br>Cercle</button>
    <button class="seq-btn" onclick="sendCmd('S3')">S3<br>Fleche Nord</button>
    <button class="seq-btn" onclick="sendCmd('STOP')">Stop<br>Moteurs</button>
  </div>
</div>

<div class="card">
  <h2>Profil de vitesse</h2>
  <div class="profile-grid">
    <button class="profile-btn" onclick="setProfile(1)">Lent</button>
    <button class="profile-btn active" onclick="setProfile(2)">Normal</button>
    <button class="profile-btn" onclick="setProfile(3)">Rapide</button>
  </div>
</div>

<div class="card">
  <h2>Commande manuelle</h2>
  <div class="manual-row">
    <select id="man-dir">
      <option value="FWD">Avancer</option>
      <option value="BCK">Reculer</option>
      <option value="TL">Gauche</option>
      <option value="TR">Droite</option>
    </select>
    <input type="number" id="man-pwm" value="190" min="0" max="255">
    <input type="number" id="man-ms" value="500" min="0">
    <button class="send-btn" onclick="sendManual()">Go</button>
  </div>
</div>

<div class="card">
  <h2>Journal</h2>
  <div id="log"></div>
</div>

<script>
let joystickActive = false;
let joySendInterval = null;
let joyX = 0, joyY = 0;
let dpadInterval = null;
let dpadCmd = null;

function log(msg, type='') {
  const d = document.getElementById('log');
  const p = document.createElement('p');
  if (type) p.className = type;
  p.textContent = new Date().toLocaleTimeString() + ' ' + msg;
  d.prepend(p);
  while (d.children.length > 20) d.removeChild(d.lastChild);
}

async function sendCmd(cmd) {
  try {
    const r = await fetch('/cmd?c=' + encodeURIComponent(cmd));
    const t = await r.text();
    log(cmd + ' -> ' + t.trim(), t.includes('ERR') ? 'err' : 'ok');
    return t;
  } catch(e) {
    log('Erreur reseau', 'err');
    return '';
  }
}

function getProfilePwm() {
  const active = document.querySelector('.profile-btn.active');
  const idx = active ? Array.prototype.indexOf.call(active.parentNode.children, active) : 1;
  return [130, 190, 230][idx];
}

const DPAD_MAP = {
  fwd: () => `FWD,${getProfilePwm()},300`,
  bck: () => `BCK,${getProfilePwm()},300`,
  tl:  () => `TL,${getProfilePwm()},300`,
  tr:  () => `TR,${getProfilePwm()},300`
};

function dpadPress(dir) {
  if (dpadInterval) return;
  document.getElementById('btn-' + dir).classList.add('pressed');
  dpadCmd = dir;
  sendCmd(DPAD_MAP[dir]());
  dpadInterval = setInterval(() => sendCmd(DPAD_MAP[dir]()), 350);
}

function dpadRelease() {
  if (dpadInterval) clearInterval(dpadInterval);
  dpadInterval = null;
  if (dpadCmd) document.getElementById('btn-' + dpadCmd).classList.remove('pressed');
  dpadCmd = null;
  sendCmd('STOP');
}

const jWrap = document.getElementById('joystick-wrap');
const jKnob = document.getElementById('joystick-knob');
const R = 90, KR = 30;

function joyMove(cx, cy) {
  const rect = jWrap.getBoundingClientRect();
  let dx = cx - (rect.left + R);
  let dy = cy - (rect.top + R);
  const dist = Math.sqrt(dx * dx + dy * dy);
  if (dist > R - KR) { dx *= (R - KR) / dist; dy *= (R - KR) / dist; }
  jKnob.style.left = (R + dx) + 'px';
  jKnob.style.top = (R + dy) + 'px';
  joyX = Math.round(dx / (R - KR) * 100);
  joyY = Math.round(-dy / (R - KR) * 100);
  document.getElementById('joy-values').textContent = `x: ${joyX} | y: ${joyY}`;
}

function joyReset() {
  jKnob.style.left = R + 'px';
  jKnob.style.top = R + 'px';
  joyX = 0; joyY = 0;
  document.getElementById('joy-values').textContent = 'x: 0 | y: 0';
  sendCmd('STOP');
}

jWrap.addEventListener('pointermove', e => { if (e.buttons || e.pointerType === 'touch') joyMove(e.clientX, e.clientY); });
jWrap.addEventListener('pointerup', joyReset);
jWrap.addEventListener('pointerleave', () => { if (joystickActive) joyReset(); });

function toggleJoystick() {
  joystickActive = !joystickActive;
  const btn = document.getElementById('joy-toggle');
  btn.classList.toggle('active', joystickActive);
  btn.textContent = joystickActive ? 'Desactiver joystick' : 'Activer joystick';
  if (joystickActive) {
    sendCmd('JSTART');
    joySendInterval = setInterval(() => sendCmd(`J,${joyX},${joyY}`), 120);
  } else {
    clearInterval(joySendInterval);
    sendCmd('JSTOP');
    joyReset();
  }
}

function setProfile(n) {
  const btns = document.querySelectorAll('.profile-btn');
  btns.forEach((b, i) => b.classList.toggle('active', i === n - 1));
  sendCmd('PROFILE,' + n).then(() => {
    document.getElementById('profile-badge').textContent = ['Lent','Normal','Rapide'][n - 1];
  });
}

function sendManual() {
  const dir = document.getElementById('man-dir').value;
  const pwm = document.getElementById('man-pwm').value;
  const ms = document.getElementById('man-ms').value;
  sendCmd(`${dir},${pwm},${ms}`);
}
</script>
</body>
</html>
)rawhtml";

static void updateLed() {
  digitalWrite(WIFI_LED_PIN, wifiActive ? HIGH : ((millis() / 500) % 2));
}

static void sendOK(const String& msg) {
  server.send(200, "text/plain", msg);
}

static bool parseTwoInts(const String& cmd, int& a, unsigned long& b) {
  const int c1 = cmd.indexOf(',');
  const int c2 = cmd.indexOf(',', c1 + 1);
  if (c1 == -1 || c2 == -1) return false;

  a = cmd.substring(c1 + 1, c2).toInt();
  b = (unsigned long)cmd.substring(c2 + 1).toInt();
  return true;
}

void handleCommand(const String& rawCmd, String& response) {
  String cmd = rawCmd;
  cmd.trim();

  if (cmd.length() == 0) {
    response = "ERR commande vide";
    return;
  }

  DBG2("[CMD] ", cmd);

  if (cmd.equalsIgnoreCase("STOP")) {
    stopMotors();
    response = "OK STOP";
    return;
  }

  if (cmd.equalsIgnoreCase("JSTART")) {
    response = "OK joystick start";
    return;
  }

  if (cmd.equalsIgnoreCase("JSTOP")) {
    stopMotors();
    response = "OK joystick stop";
    return;
  }

  if (cmd.startsWith("J,")) {
    const int c1 = cmd.indexOf(',');
    const int c2 = cmd.indexOf(',', c1 + 1);
    if (c1 == -1 || c2 == -1) {
      response = "ERR format J,x,y";
      return;
    }

    const int x = constrain(cmd.substring(c1 + 1, c2).toInt(), -100, 100);
    const int y = constrain(cmd.substring(c2 + 1).toInt(), -100, 100);

    if (abs(x) < 10 && abs(y) < 10) {
      stopMotors();
      response = "OK joystick stop";
      return;
    }

    const int speed = map(abs(y), 0, 100, 0, activeProfile.pwm_straight);
    const int turn = map(abs(x), 0, 100, 0, activeProfile.pwm_turn / 2);
    const int leftPwm = constrain(speed + (x < 0 ? turn : -turn), 0, PWM_MAX);
    const int rightPwm = constrain(speed + (x < 0 ? -turn : turn), 0, PWM_MAX);
    const int direction = (y >= 0) ? FORWARD : BACKWARD;

    setMotorPower(LEFT_MOTOR, direction, leftPwm);
    setMotorPower(RIGHT_MOTOR, direction, rightPwm);
    response = "OK joystick";
    return;
  }

  if (cmd.equalsIgnoreCase("S1")) {
    sequenceEscalier();
    response = "OK S1 escalier";
    return;
  }

  if (cmd.equalsIgnoreCase("S2") || cmd.startsWith("S2,")) {
    int radius = 20;
    if (cmd.startsWith("S2,")) radius = cmd.substring(3).toInt();
    sequenceCircle(radius);
    response = "OK S2 cercle";
    return;
  }

  if (cmd.equalsIgnoreCase("S3")) {
    sequenceNorthArrow();
    response = "OK S3 fleche nord";
    return;
  }

  int pwm = 0;
  unsigned long ms = 0;

  if (cmd.startsWith("FWD,")) {
    if (parseTwoInts(cmd, pwm, ms)) {
      moveRobotStraight(FORWARD, pwm, ms);
      response = "OK fwd";
    } else {
      response = "ERR format FWD,pwm,ms";
    }
    return;
  }

  if (cmd.startsWith("BCK,")) {
    if (parseTwoInts(cmd, pwm, ms)) {
      moveRobotStraight(BACKWARD, pwm, ms);
      response = "OK bck";
    } else {
      response = "ERR format BCK,pwm,ms";
    }
    return;
  }

  if (cmd.startsWith("TL,")) {
    if (parseTwoInts(cmd, pwm, ms)) {
      turnRobot(TURN_LEFT, pwm, ms);
      response = "OK tl";
    } else {
      response = "ERR format TL,pwm,ms";
    }
    return;
  }

  if (cmd.startsWith("TR,")) {
    if (parseTwoInts(cmd, pwm, ms)) {
      turnRobot(TURN_RIGHT, pwm, ms);
      response = "OK tr";
    } else {
      response = "ERR format TR,pwm,ms";
    }
    return;
  }

  if (cmd.startsWith("PROFILE,")) {
    const int idx = cmd.substring(8).toInt() - 1;
    if (idx >= 0 && idx < PROFILE_COUNT) {
      activeProfileIdx = idx;
      activeProfile = PROFILES[idx];
      response = "OK profil ";
      response += activeProfile.name;
    } else {
      response = "ERR profil invalide";
    }
    return;
  }

  if (cmd.equalsIgnoreCase("PROFILES")) {
    response = "";
    for (int i = 0; i < PROFILE_COUNT; i++) {
      response += String(i + 1);
      response += (i == activeProfileIdx) ? "[*] " : "    ";
      response += PROFILES[i].name;
      response += "\n";
    }
    return;
  }

  if (cmd.equalsIgnoreCase("STATUS")) {
    response = "Profil: ";
    response += String(activeProfileIdx + 1);
    response += " - ";
    response += activeProfile.name;
    response += "\nWiFi: ";
    response += wifiActive ? "ON" : "OFF";
    response += "\nIP: ";
    response += WIFI_AP_IP;
    return;
  }

  response = "ERR commande inconnue";
}

static void routeRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

static void routeCmd() {
  if (!server.hasArg("c")) {
    server.send(400, "text/plain", "ERR missing ?c=");
    return;
  }

  String response;
  handleCommand(server.arg("c"), response);
  sendOK(response);
}

static void routeStatus() {
  String response;
  handleCommand("STATUS", response);
  sendOK(response);
}

void enableWifi() {
  if (wifiActive) return;

  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  ip.fromString(WIFI_AP_IP);
  gateway.fromString(WIFI_AP_GATEWAY);
  subnet.fromString(WIFI_AP_SUBNET);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
  delay(100);

  server.on("/", HTTP_GET, routeRoot);
  server.on("/cmd", HTTP_GET, routeCmd);
  server.on("/status", HTTP_GET, routeStatus);
  server.begin();

  wifiActive = true;
  digitalWrite(WIFI_LED_PIN, HIGH);

  DBG("[WiFi] Point d'acces demarre");
  DBG2("[WiFi] IP: ", WiFi.softAPIP().toString());
}

void disableWifi() {
  if (!wifiActive) return;

  stopMotors();
  server.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiActive = false;
  digitalWrite(WIFI_LED_PIN, LOW);

  DBG("[WiFi] Desactive");
}

void setupWifi() {
  pinMode(WIFI_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW);

  // Le robot doit etre utilisable tout de suite apres le boot.
  enableWifi();
  DBG("[WiFi] BOOT permet de couper ou relancer le WiFi");
}

void loopWifi() {
  if (digitalRead(WIFI_TOGGLE_PIN) == LOW) {
    const unsigned long now = millis();
    if (now - lastButtonPress > DEBOUNCE_MS) {
      lastButtonPress = now;
      if (wifiActive) disableWifi();
      else enableWifi();
    }
  }

  updateLed();

  if (wifiActive) {
    server.handleClient();
  }
}
