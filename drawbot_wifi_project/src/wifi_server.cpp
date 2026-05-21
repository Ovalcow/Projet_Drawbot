#include "wifi_server.h"
#include "motors.h"
#include "sequences.h"
#include "config.h"
#include <WiFi.h>
#include <WebServer.h>

// -------------------------------------------------------
//  État global
// -------------------------------------------------------
RobotProfile activeProfile    = PROFILES[DEFAULT_PROFILE_INDEX];
static int   activeProfileIdx = DEFAULT_PROFILE_INDEX;
static bool  wifiActive       = false;

static WebServer server(HTTP_PORT);

// Anti-rebond bouton BOOT
static unsigned long lastButtonPress = 0;
static const unsigned long DEBOUNCE_MS = 300;

// -------------------------------------------------------
//  LED helpers
// -------------------------------------------------------
static void updateLed() {
  if (wifiActive) {
    digitalWrite(WIFI_LED_PIN, HIGH);
  } else {
    digitalWrite(WIFI_LED_PIN, (millis() / 500) % 2);
  }
}

// -------------------------------------------------------
//  Page HTML embarquée
// -------------------------------------------------------
static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Drawbot</title>
<style>
  :root {
    --bg: #0f0f13; --surface: #1a1a24; --border: #2e2e42;
    --accent: #6c63ff; --accent2: #ff6584; --text: #e8e8f0;
    --muted: #888; --green: #4ade80; --red: #f87171;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { background: var(--bg); color: var(--text); font-family: system-ui, sans-serif;
         min-height: 100vh; padding: 16px; }
  h1 { text-align: center; font-size: 1.4rem; letter-spacing: 3px;
       color: var(--accent); margin-bottom: 4px; }
  .subtitle { text-align: center; font-size: .75rem; color: var(--muted); margin-bottom: 20px; }
  .card { background: var(--surface); border: 1px solid var(--border);
          border-radius: 14px; padding: 16px; margin-bottom: 14px; }
  .card h2 { font-size: .8rem; letter-spacing: 2px; color: var(--muted);
             text-transform: uppercase; margin-bottom: 12px; }

  /* Status bar */
  #status-bar { display:flex; align-items:center; gap:8px; font-size:.85rem; }
  #status-dot { width:10px; height:10px; border-radius:50%; background:var(--green); flex-shrink:0; }
  #status-txt { flex:1; }
  #profile-badge { background:var(--accent); border-radius:20px;
                   padding:2px 10px; font-size:.75rem; }

  /* Direction pad */
  .dpad { display:grid; grid-template-columns:repeat(3,1fr); gap:8px;
          max-width:220px; margin:0 auto; }
  .dpad-btn { aspect-ratio:1; border:none; border-radius:12px;
              background:var(--border); color:var(--text);
              font-size:1.4rem; cursor:pointer; transition:.15s;
              display:flex; align-items:center; justify-content:center; }
  .dpad-btn:active, .dpad-btn.pressed { background:var(--accent); transform:scale(.93); }
  .dpad-btn.stop { background:#2a1a2a; color:var(--red); font-size:1rem; }
  .dpad-btn.empty { background:transparent; pointer-events:none; }

  /* Joystick */
  #joystick-area { display:flex; flex-direction:column; align-items:center; gap:10px; }
  #joystick-wrap { position:relative; width:180px; height:180px;
                   background:var(--border); border-radius:50%;
                   border:2px solid var(--accent); touch-action:none; }
  #joystick-knob { position:absolute; width:60px; height:60px;
                   background:var(--accent); border-radius:50%;
                   top:50%; left:50%; transform:translate(-50%,-50%);
                   pointer-events:none; transition:background .1s; }
  #joy-values { font-size:.75rem; color:var(--muted); }
  #joy-toggle { padding:8px 20px; border:1px solid var(--accent); border-radius:20px;
                background:transparent; color:var(--accent); cursor:pointer; font-size:.85rem; }
  #joy-toggle.active { background:var(--accent); color:#fff; }

  /* Séquences */
  .seq-grid { display:grid; grid-template-columns:repeat(2,1fr); gap:8px; }
  .seq-btn { padding:12px 6px; border:1px solid var(--border); border-radius:10px;
             background:transparent; color:var(--text); cursor:pointer; font-size:.8rem;
             text-align:center; transition:.15s; }
  .seq-btn:active { background:var(--accent2); border-color:var(--accent2); }

  /* Profils */
  .profile-grid { display:grid; grid-template-columns:repeat(3,1fr); gap:8px; }
  .profile-btn { padding:10px 4px; border:1px solid var(--border); border-radius:10px;
                 background:transparent; color:var(--muted); cursor:pointer;
                 font-size:.8rem; text-align:center; transition:.15s; }
  .profile-btn.active { border-color:var(--accent); color:var(--text); background:#2a2a3a; }

  /* Commande manuelle */
  .manual-row { display:flex; gap:8px; align-items:center; flex-wrap:wrap; }
  .manual-row input { flex:1; min-width:60px; padding:8px; border-radius:8px;
                      border:1px solid var(--border); background:#0f0f1a;
                      color:var(--text); font-size:.85rem; }
  .manual-row select { padding:8px; border-radius:8px; border:1px solid var(--border);
                       background:#0f0f1a; color:var(--text); font-size:.85rem; }
  .send-btn { padding:8px 16px; border:none; border-radius:8px;
              background:var(--accent); color:#fff; cursor:pointer; font-size:.85rem; }

  /* Log */
  #log { font-size:.72rem; color:var(--muted); font-family:monospace;
         max-height:80px; overflow-y:auto; }
  #log p { margin:1px 0; }
  #log p.ok  { color:var(--green); }
  #log p.err { color:var(--red); }
</style>
</head>
<body>
<h1>⬡ DRAWBOT</h1>
<p class="subtitle">Interface de contrôle WiFi</p>

<!-- Status -->
<div class="card">
  <div id="status-bar">
    <div id="status-dot"></div>
    <span id="status-txt">Connecté</span>
    <span id="profile-badge">Normal</span>
  </div>
</div>

<!-- Direction pad -->
<div class="card">
  <h2>Déplacement</h2>
  <div class="dpad">
    <div class="dpad-btn empty"></div>
    <button class="dpad-btn" id="btn-fwd"  ontouchstart="dpadPress('fwd')"  ontouchend="dpadRelease()" onmousedown="dpadPress('fwd')"  onmouseup="dpadRelease()">▲</button>
    <div class="dpad-btn empty"></div>
    <button class="dpad-btn" id="btn-tl"   ontouchstart="dpadPress('tl')"   ontouchend="dpadRelease()" onmousedown="dpadPress('tl')"   onmouseup="dpadRelease()">◀</button>
    <button class="dpad-btn stop"           onclick="sendCmd('STOP')">■</button>
    <button class="dpad-btn" id="btn-tr"   ontouchstart="dpadPress('tr')"   ontouchend="dpadRelease()" onmousedown="dpadPress('tr')"   onmouseup="dpadRelease()">▶</button>
    <div class="dpad-btn empty"></div>
    <button class="dpad-btn" id="btn-bck"  ontouchstart="dpadPress('bck')"  ontouchend="dpadRelease()" onmousedown="dpadPress('bck')"  onmouseup="dpadRelease()">▼</button>
    <div class="dpad-btn empty"></div>
  </div>
</div>

<!-- Joystick -->
<div class="card">
  <h2>Joystick</h2>
  <div id="joystick-area">
    <div id="joystick-wrap">
      <div id="joystick-knob"></div>
    </div>
    <div id="joy-values">x: 0 | y: 0</div>
    <button id="joy-toggle" onclick="toggleJoystick()">Activer joystick</button>
  </div>
</div>

<!-- Modes -->
<div class="card">
  <h2>Modes</h2>
  <div class="seq-grid">
    <button class="seq-btn" onclick="toggleJoystick()">🎮<br>Libre</button>
    <button class="seq-btn" onclick="sendCmd('S1')">🪜<br>Escalier</button>
    <button class="seq-btn" onclick="sendCmd('S2')">⬜<br>Carré</button>
    <button class="seq-btn" onclick="sendCmd('S3')">〰️<br>Zigzag</button>
  </div>
</div>

<!-- Profils -->
<div class="card">
  <h2>Profil de vitesse</h2>
  <div class="profile-grid" id="profile-grid">
    <button class="profile-btn" onclick="setProfile(1)">🐢<br>Lent</button>
    <button class="profile-btn active" onclick="setProfile(2)">🚗<br>Normal</button>
    <button class="profile-btn" onclick="setProfile(3)">🚀<br>Rapide</button>
  </div>
</div>

<!-- Commande manuelle -->
<div class="card">
  <h2>Commande manuelle</h2>
  <div class="manual-row">
    <select id="man-dir">
      <option value="FWD">Avancer</option>
      <option value="BCK">Reculer</option>
      <option value="TL">Tourner G</option>
      <option value="TR">Tourner D</option>
    </select>
    <input type="number" id="man-pwm" placeholder="PWM" value="190" min="0" max="255">
    <input type="number" id="man-ms"  placeholder="ms"  value="500" min="0">
    <button class="send-btn" onclick="sendManual()">Go</button>
  </div>
</div>

<!-- Log -->
<div class="card">
  <h2>Journal</h2>
  <div id="log"></div>
</div>

<script>
// ---- Utilitaires ----
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
    log(cmd + ' → ' + t.trim(), t.includes('ERR') ? 'err' : 'ok');
  } catch(e) { log('Erreur réseau', 'err'); }
}

// ---- D-pad (maintien = envois répétés) ----
const DPAD_MAP = {
  fwd: () => `FWD,${getProfilePwm()},300`,
  bck: () => `BCK,${getProfilePwm()},300`,
  tl:  () => `TL,${getProfilePwm()},300`,
  tr:  () => `TR,${getProfilePwm()},300`,
};

function getProfilePwm() {
  const active = document.querySelector('.profile-btn.active');
  const idx = active ? [...active.parentNode.children].indexOf(active) : 1;
  return [130, 190, 230][idx];
}

function dpadPress(dir) {
  if (dpadInterval) return;
  document.getElementById('btn-' + dir)?.classList.add('pressed');
  dpadCmd = dir;
  sendCmd(DPAD_MAP[dir]());
  dpadInterval = setInterval(() => sendCmd(DPAD_MAP[dir]()), 350);
}

function dpadRelease() {
  if (dpadInterval) { clearInterval(dpadInterval); dpadInterval = null; }
  if (dpadCmd) document.getElementById('btn-' + dpadCmd)?.classList.remove('pressed');
  dpadCmd = null;
  sendCmd('STOP');
}

// ---- Joystick ----
const jWrap = document.getElementById('joystick-wrap');
const jKnob = document.getElementById('joystick-knob');
const R = 90, KR = 30; // rayon zone, rayon knob

function joyMove(cx, cy) {
  const rect = jWrap.getBoundingClientRect();
  let dx = cx - (rect.left + R);
  let dy = cy - (rect.top  + R);
  const dist = Math.sqrt(dx*dx + dy*dy);
  if (dist > R - KR) { dx *= (R-KR)/dist; dy *= (R-KR)/dist; }
  jKnob.style.left = (R + dx) + 'px';
  jKnob.style.top  = (R + dy) + 'px';
  joyX = Math.round(dx / (R - KR) * 100);
  joyY = Math.round(-dy / (R - KR) * 100);
  document.getElementById('joy-values').textContent = `x: ${joyX} | y: ${joyY}`;
}

function joyReset() {
  jKnob.style.left = R + 'px'; jKnob.style.top = R + 'px';
  joyX = 0; joyY = 0;
  document.getElementById('joy-values').textContent = 'x: 0 | y: 0';
  sendCmd('STOP');
}

jWrap.addEventListener('touchmove', e => { e.preventDefault(); joyMove(e.touches[0].clientX, e.touches[0].clientY); }, {passive:false});
jWrap.addEventListener('touchend',  () => joyReset());
jWrap.addEventListener('mousemove', e => { if (e.buttons) joyMove(e.clientX, e.clientY); });
jWrap.addEventListener('mouseup',   () => joyReset());
jWrap.addEventListener('mouseleave',() => { if (joystickActive) joyReset(); });

function toggleJoystick() {
  joystickActive = !joystickActive;
  const btn = document.getElementById('joy-toggle');
  btn.classList.toggle('active', joystickActive);
  btn.textContent = joystickActive ? 'Désactiver joystick' : 'Activer joystick';
  if (joystickActive) {
    sendCmd('JSTART');
    joySendInterval = setInterval(() => sendCmd(`J,${joyX},${joyY}`), 100);
  } else {
    clearInterval(joySendInterval);
    sendCmd('JSTOP');
    joyReset();
  }
}

// ---- Profils ----
function setProfile(n) {
  const btns = document.querySelectorAll('.profile-btn');
  btns.forEach((b, i) => b.classList.toggle('active', i === n-1));
  sendCmd('PROFILE,' + n).then(() => {
    const names = ['Lent','Normal','Rapide'];
    document.getElementById('profile-badge').textContent = names[n-1];
  });
}

// ---- Commande manuelle ----
function sendManual() {
  const dir = document.getElementById('man-dir').value;
  const pwm = document.getElementById('man-pwm').value;
  const ms  = document.getElementById('man-ms').value;
  sendCmd(`${dir},${pwm},${ms}`);
}
</script>
</body>
</html>
)rawhtml";

// -------------------------------------------------------
//  Helpers de réponse HTTP
// -------------------------------------------------------
static void sendOK(const String& msg) {
  server.send(200, "text/plain", msg);
}

// -------------------------------------------------------
//  Gestion des commandes (logique identique à l'ancienne BT)
// -------------------------------------------------------
void handleCommand(const String& rawCmd, String& response) {
  String cmd = rawCmd;
  cmd.trim();
  if (cmd.length() == 0) { response = "empty"; return; }
  DBG2("[CMD] ", cmd);

  // STOP
  if (cmd.equalsIgnoreCase("STOP")) {
    stopMotors();
    response = "OK STOP"; return;
  }

  // Mode joystick on/off
  if (cmd.equalsIgnoreCase("JSTART")) { response = "OK joystick start"; return; }
  if (cmd.equalsIgnoreCase("JSTOP"))  { stopMotors(); response = "OK joystick stop"; return; }

  // Joystick J,x,y
  if (cmd.startsWith("J,")) {
    int c1 = cmd.indexOf(',');
    int c2 = cmd.indexOf(',', c1+1);
    if (c1 == -1 || c2 == -1) { response = "ERR format J,x,y"; return; }
    int x = cmd.substring(c1+1, c2).toInt();
    int y = cmd.substring(c2+1).toInt();
    if (abs(x) < 10 && abs(y) < 10) { stopMotors(); response = "OK stop"; return; }
    int spd  = map(abs(y), 0, 100, 0, activeProfile.pwm_straight);
    int turn = map(abs(x), 0, 100, 0, activeProfile.pwm_turn / 2);
    int lp   = constrain(spd + (x < 0 ?  turn : -turn), 0, PWM_MAX);
    int rp   = constrain(spd + (x < 0 ? -turn :  turn), 0, PWM_MAX);
    int dir  = (y >= 0) ? FORWARD : BACKWARD;
    setMotorPower(LEFT_MOTOR,  dir, lp);
    setMotorPower(RIGHT_MOTOR, dir, rp);
    response = "OK J"; return;
  }

  // Séquences
  if (cmd.equalsIgnoreCase("S1")) { sequenceEscalier(); response = "OK escalier"; return; }
  if (cmd.equalsIgnoreCase("S2")) { sequenceCarre();    response = "OK carre";    return; }
  if (cmd.equalsIgnoreCase("S3")) { sequenceZigzag();   response = "OK zigzag";   return; }

  // Helper parse deux entiers "CMD,a,b"
  auto parseTwoInts = [&](int& a, unsigned long& b) -> bool {
    int c1 = cmd.indexOf(',');
    int c2 = cmd.indexOf(',', c1+1);
    if (c1 == -1 || c2 == -1) return false;
    a = cmd.substring(c1+1, c2).toInt();
    b = (unsigned long)cmd.substring(c2+1).toInt();
    return true;
  };

  if (cmd.startsWith("FWD,")) {
    int pwm; unsigned long ms;
    if (parseTwoInts(pwm, ms)) { moveRobotStraight(FORWARD,  pwm, ms); response = "OK fwd"; }
    else response = "ERR format FWD,pwm,ms";
    return;
  }
  if (cmd.startsWith("BCK,")) {
    int pwm; unsigned long ms;
    if (parseTwoInts(pwm, ms)) { moveRobotStraight(BACKWARD, pwm, ms); response = "OK bck"; }
    else response = "ERR format BCK,pwm,ms";
    return;
  }
  if (cmd.startsWith("TL,")) {
    int pwm; unsigned long ms;
    if (parseTwoInts(pwm, ms)) { turnRobot(TURN_LEFT,  pwm, ms); response = "OK tl"; }
    else response = "ERR format TL,pwm,ms";
    return;
  }
  if (cmd.startsWith("TR,")) {
    int pwm; unsigned long ms;
    if (parseTwoInts(pwm, ms)) { turnRobot(TURN_RIGHT, pwm, ms); response = "OK tr"; }
    else response = "ERR format TR,pwm,ms";
    return;
  }

  // Profils
  if (cmd.startsWith("PROFILE,")) {
    int idx = cmd.substring(8).toInt() - 1;
    if (idx >= 0 && idx < PROFILE_COUNT) {
      activeProfileIdx = idx;
      activeProfile    = PROFILES[idx];
      response = "OK profil " + String(activeProfile.name);
    } else {
      response = "ERR profil invalide";
    }
    return;
  }
  if (cmd.equalsIgnoreCase("PROFILES")) {
    response = "";
    for (int i = 0; i < PROFILE_COUNT; i++) {
      response += String(i+1) + (i == activeProfileIdx ? "[*]" : "   ");
      response += String(PROFILES[i].name) + "\n";
    }
    return;
  }

  // Statut
  if (cmd.equalsIgnoreCase("STATUS")) {
    response  = "Profil: " + String(activeProfileIdx+1) + " - " + activeProfile.name + "\n";
    response += "WiFi: ON\n";
    return;
  }

  response = "ERR commande inconnue";
}

// -------------------------------------------------------
//  Routes HTTP
// -------------------------------------------------------
static void routeRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

static void routeCmd() {
  if (!server.hasArg("c")) { server.send(400, "text/plain", "ERR missing ?c="); return; }
  String response;
  handleCommand(server.arg("c"), response);
  sendOK(response);
}

static void routeStatus() {
  String response;
  handleCommand("STATUS", response);
  sendOK(response);
}

// -------------------------------------------------------
void enableWifi() {
  if (wifiActive) return;
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
  delay(100);
  DBG2("[WiFi] AP IP: ", WiFi.softAPIP().toString());

  server.on("/",       HTTP_GET, routeRoot);
  server.on("/cmd",    HTTP_GET, routeCmd);
  server.on("/status", HTTP_GET, routeStatus);
  server.begin();

  wifiActive = true;
  digitalWrite(WIFI_LED_PIN, HIGH);
  DBG("[WiFi] Serveur HTTP démarré.");
  DBG2("[WiFi] Ouvre : http://", WiFi.softAPIP().toString());
}

void disableWifi() {
  if (!wifiActive) return;
  stopMotors();
  server.stop();
  WiFi.softAPdisconnect(true);
  wifiActive = false;
  digitalWrite(WIFI_LED_PIN, LOW);
  DBG("[WiFi] Désactivé.");
}

// -------------------------------------------------------
void setupWifi() {
  pinMode(WIFI_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(WIFI_LED_PIN,    OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW);
  DBG("[WiFi] Appuyer sur BOOT (GPIO0) pour activer le WiFi.");
}

void loopWifi() {
  // Bouton BOOT avec anti-rebond
  if (digitalRead(WIFI_TOGGLE_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastButtonPress > DEBOUNCE_MS) {
      lastButtonPress = now;
      if (!wifiActive) enableWifi();
      else             disableWifi();
    }
  }

  updateLed();

  if (wifiActive) server.handleClient();
}
