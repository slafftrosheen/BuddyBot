#pragma once
#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>BuddyBot Web UI</title>
<style>
:root {
  --bg-color: #121212;
  --panel-bg: rgba(255, 255, 255, 0.05);
  --text-main: #E0E0E0;
  --text-dim: #888888;
  --accent: #BB86FC;
  --accent-hover: #D0A0FD;
  --danger: #CF6679;
  --danger-hover: #E57373;
  --success: #03DAC6;
  --border: rgba(255, 255, 255, 0.1);
}
* { box-sizing: border-box; font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; }
body {
  margin: 0; padding: 10px; background-color: var(--bg-color); color: var(--text-main);
  overscroll-behavior: none; user-select: none; -webkit-user-select: none;
}
.container { max-width: 500px; margin: 0 auto; display: flex; flex-direction: column; gap: 15px; }
.panel {
  background: var(--panel-bg); border: 1px solid var(--border); border-radius: 12px;
  padding: 15px; backdrop-filter: blur(10px);
}
h2 { margin: 0 0 10px 0; font-size: 1.1em; color: var(--accent); }
.badge-row { display: flex; justify-content: space-between; font-size: 0.9em; }
.status-badge { padding: 4px 8px; border-radius: 6px; background: rgba(255,255,255,0.1); }
.status-badge.connected { background: var(--success); color: #000; font-weight: bold; }
.status-badge.disconnected { background: var(--danger); color: #fff; font-weight: bold; }

.btn {
  background: rgba(255,255,255,0.1); border: 1px solid var(--border); color: var(--text-main);
  border-radius: 8px; padding: 10px; cursor: pointer; transition: 0.2s; font-size: 1em;
  display: flex; justify-content: center; align-items: center; touch-action: manipulation;
}
.btn:active { background: rgba(255,255,255,0.2); transform: scale(0.95); }
.btn.stop { background: var(--danger); color: white; font-weight: bold; height: 60px; font-size: 1.2em; border: none; }
.btn.stop:active { background: var(--danger-hover); }

.dpad {
  display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin-top: 10px;
}
.dpad .btn { height: 60px; }
.grid-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
.grid-3 { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; }

#loginModal {
  display: none; position: fixed; inset: 0; background: rgba(0,0,0,0.8);
  justify-content: center; align-items: center; z-index: 100;
}
#loginModal .panel { width: 300px; text-align: center; }
input { width: 100%; padding: 10px; margin-bottom: 10px; border-radius: 6px; border: 1px solid var(--border); background: var(--bg-color); color: var(--text-main); text-align: center; font-size: 1.2em; }

.log { height: 100px; overflow-y: auto; font-family: monospace; font-size: 0.8em; color: var(--text-dim); background: #000; padding: 5px; border-radius: 6px; }
.log div { margin-bottom: 2px; }
.log .err { color: var(--danger); }
.log .ok { color: var(--success); }

.armed { border-color: var(--danger); }
.disarmed { border-color: var(--border); }
</style>
</head>
<body>
  <div id="loginModal">
    <div class="panel">
      <h2>Take Control</h2>
      <p>Enter PIN to take control of BuddyBot</p>
      <input type="number" id="pinInput" placeholder="PIN" pattern="[0-9]*">
      <button class="btn" style="width: 100%; background: var(--accent); color: #000;" onclick="submitPin()">Submit</button>
      <button class="btn" style="width: 100%; margin-top: 10px;" onclick="closeModal()">Close (View Only)</button>
    </div>
  </div>

  <div class="container">
    <div class="panel">
      <div class="badge-row">
        <span>WebUI</span>
        <span id="wsBadge" class="status-badge disconnected">Disconnected</span>
      </div>
      <div class="badge-row" style="margin-top: 8px;">
        <span>Role</span>
        <span id="roleBadge">Observer</span>
        <button id="authBtn" onclick="openModal()" style="background:none;border:none;color:var(--accent);text-decoration:underline;">Take Control</button>
      </div>
      <div class="badge-row" style="margin-top: 8px;">
        <span>Range</span>
        <span id="rangeBadge">-- mm</span>
      </div>
    </div>

    <button class="btn stop" id="btnStop" onpointerdown="sendStop()">EMERGENCY STOP</button>

    <div class="panel" id="motorPanel">
      <h2>Drive Control <span id="armedStatus" style="font-size:0.8em;float:right;">Disarmed</span></h2>
      <button class="btn" id="btnArm" onclick="toggleArm()">Toggle Arm Motors</button>
      <div class="dpad">
        <div></div>
        <button class="btn" data-mode="forward">FWD</button>
        <div></div>
        <button class="btn" data-mode="turn_left">LEFT</button>
        <button class="btn" data-mode="reverse">REV</button>
        <button class="btn" data-mode="turn_right">RIGHT</button>
      </div>
    </div>

    <div class="panel">
      <h2>Actions</h2>
      <div class="grid-3">
        <button class="btn" onclick="sendAction('wave')">Wave</button>
        <button class="btn" onclick="sendAction('look_left')">L Left</button>
        <button class="btn" onclick="sendAction('look_right')">L Right</button>
        <button class="btn" onclick="sendAction('greet')">Greet</button>
        <button class="btn" onclick="sendAction('celebrate')">Celebrate</button>
        <button class="btn" onclick="sendAction('dance')">Dance</button>
        <button class="btn" onclick="sendAction('sleep')">Sleep</button>
      </div>
    </div>

    <div class="panel">
      <h2>Moods</h2>
      <div class="grid-3">
        <button class="btn" onclick="sendMood('idle')">Idle</button>
        <button class="btn" onclick="sendMood('happy')">Happy</button>
        <button class="btn" onclick="sendMood('curious')">Curious</button>
        <button class="btn" onclick="sendMood('sleepy')">Sleepy</button>
        <button class="btn" onclick="sendMood('excited')">Excited</button>
        <button class="btn" onclick="sendMood('alert')">Alert</button>
      </div>
    </div>
    
    <div class="panel">
      <h2>Accessories</h2>
      <div class="grid-3">
        <button class="btn" onclick="toggleAccessory(1)">Acc 1</button>
        <button class="btn" onclick="toggleAccessory(2)">Acc 2</button>
        <button class="btn" onclick="toggleAccessory(3)">Acc 3</button>
      </div>
    </div>

    <div class="panel">
      <h2>Event Log</h2>
      <div id="log" class="log"></div>
    </div>
  </div>

<script>
let ws;
let isController = false;
let driveInterval = null;
let currentDriveMode = null;
let pin = "";
let accState = {1: false, 2: false, 3: false};
let msgId = 1;

const logEl = document.getElementById('log');
function logMsg(msg, isErr=false, isOk=false) {
  const div = document.createElement('div');
  div.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
  if (isErr) div.className = 'err';
  if (isOk) div.className = 'ok';
  logEl.prepend(div);
}

function openModal() { document.getElementById('loginModal').style.display = 'flex'; }
function closeModal() { document.getElementById('loginModal').style.display = 'none'; }
function submitPin() {
  pin = document.getElementById('pinInput').value;
  closeModal();
  send({type: "auth", pin: pin});
}

function connect() {
  ws = new WebSocket(`ws://${window.location.host}/ws`);
  ws.onopen = () => {
    document.getElementById('wsBadge').textContent = 'Connected';
    document.getElementById('wsBadge').className = 'status-badge connected';
    logMsg('WebSocket connected');
    if (pin) send({type: "auth", pin: pin});
  };
  ws.onclose = () => {
    document.getElementById('wsBadge').textContent = 'Disconnected';
    document.getElementById('wsBadge').className = 'status-badge disconnected';
    isController = false;
    updateRoleUI();
    logMsg('WebSocket disconnected', true);
    setTimeout(connect, 2000);
  };
  ws.onmessage = (e) => {
    try {
      const data = JSON.parse(e.data);
      if (data.type === 'telemetry') {
        updateTelemetry(data);
      } else if (data.type === 'ack') {
        logMsg(`Ack ID ${data.id}: ${data.message || (data.ok ? "OK" : "Failed")}`, !data.ok, data.ok);
        if (data.auth !== undefined) {
          isController = data.auth;
          updateRoleUI();
          if (isController) logMsg("Successfully took control", false, true);
        }
      }
    } catch(err) {}
  };
}

function send(obj) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    obj.v = 1;
    obj.id = msgId++;
    ws.send(JSON.stringify(obj));
  }
}

function updateTelemetry(t) {
  document.getElementById('rangeBadge').textContent = t.rangeMm ? `${t.rangeMm} mm` : '--';
  document.getElementById('armedStatus').textContent = t.isArmed ? 'ARMED' : 'Disarmed';
  document.getElementById('armedStatus').style.color = t.isArmed ? 'var(--danger)' : 'var(--text-dim)';
  document.getElementById('motorPanel').className = t.isArmed ? 'panel armed' : 'panel disarmed';
}

function updateRoleUI() {
  document.getElementById('roleBadge').textContent = isController ? 'Controller' : 'Observer';
  document.getElementById('roleBadge').style.color = isController ? 'var(--success)' : 'inherit';
  document.getElementById('authBtn').style.display = isController ? 'none' : 'inline-block';
}

function sendStop() {
  stopDriveLoop();
  send({type: "stop"});
}

function toggleArm() {
  const willArm = document.getElementById('armedStatus').textContent === 'Disarmed';
  send({type: willArm ? "arm" : "disarm"});
}

function sendAction(actionStr) {
  send({type: "action", action: actionStr});
}

function sendMood(moodStr) {
  send({type: "mood", mood: moodStr});
}

function toggleAccessory(idx) {
  accState[idx] = !accState[idx];
  send({type: "accessory", index: idx, active: accState[idx]});
}

// Drive logic
function startDriveLoop(mode) {
  if (!isController) return;
  currentDriveMode = mode;
  sendDriveCmd();
  if (driveInterval) clearInterval(driveInterval);
  driveInterval = setInterval(sendDriveCmd, 150);
}

function sendDriveCmd() {
  if (!currentDriveMode) return;
  send({type: "move", mode: currentDriveMode, durationMs: 250});
}

function stopDriveLoop() {
  if (driveInterval) clearInterval(driveInterval);
  driveInterval = null;
  if (currentDriveMode) {
    currentDriveMode = null;
    send({type: "move", mode: "stopped", durationMs: 0});
  }
}

document.querySelectorAll('.dpad .btn').forEach(btn => {
  const mode = btn.dataset.mode;
  if (mode) {
    btn.onpointerdown = (e) => { e.preventDefault(); startDriveLoop(mode); };
    btn.onpointerup = (e) => { e.preventDefault(); stopDriveLoop(); };
    btn.onpointerleave = (e) => { e.preventDefault(); stopDriveLoop(); };
  }
});

connect();
</script>
</body>
</html>
)=====";
