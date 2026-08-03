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
      <h2>Pair Controller</h2>
      <p>Enter the code shown on BuddyBot's screen</p>
      <input type="text" id="pinInput" placeholder="1234 5678">
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
      <div class="badge-row" style="margin-top: 8px;">
        <span>Firmware</span>
        <span id="firmwareBadge">--</span>
      </div>
      <div class="badge-row" style="margin-top: 8px;">
        <span>Built-in IMU</span>
        <span id="imuBadge">--</span>
      </div>
      <div class="badge-row" style="margin-top: 8px;">
        <span>Acceleration</span>
        <span id="accelBadge">--</span>
      </div>
      <div class="badge-row" style="margin-top: 8px;">
        <span>Rotation</span>
        <span id="gyroBadge">--</span>
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
const session = {
  ws: null,
  connected: false,
  role: "disconnected", // disconnected | observer | pairing | controller | busy
  token: "",
  nextRequestId: 1,
  activeDriveTimer: 0,
  heldDriveMode: "",
  telemetryRevision: 0,
  hasDrive: true,
  hasManipulators: true,
  lastActionRunning: false,
  forwardMotionBlocked: false
};

let accState = {1: false, 2: false, 3: false};

const logEl = document.getElementById('log');
function logMsg(msg, isErr=false, isOk=false) {
  const div = document.createElement('div');
  div.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
  if (isErr) div.className = 'err';
  if (isOk) div.className = 'ok';
  logEl.prepend(div);
}

function updateRoleUI() {
  const badge = document.getElementById('roleBadge');
  const authBtn = document.getElementById('authBtn');
  
  if (session.role === 'controller') {
    badge.textContent = 'Controller';
    badge.style.color = 'var(--success)';
    authBtn.style.display = 'none';
  } else if (session.role === 'pairing') {
    badge.textContent = 'Pairing Required';
    badge.style.color = 'var(--accent)';
    authBtn.style.display = 'inline-block';
  } else if (session.role === 'busy') {
    badge.textContent = 'Controller Busy';
    badge.style.color = 'var(--danger)';
    authBtn.style.display = 'none';
  } else {
    badge.textContent = 'Observer';
    badge.style.color = 'inherit';
    authBtn.style.display = 'inline-block';
  }
  
  // Disable privileged controls if not controller, or if an action is running
  const isController = session.role === 'controller';
  const isActionRunning = session.lastActionRunning;
  
  const driveControls = document.querySelectorAll('.dpad button, #btnArm');
  driveControls.forEach(btn => {
    const active = isController && session.hasDrive && !isActionRunning;
    if (btn.dataset.mode === 'forward' && session.forwardMotionBlocked) {
      btn.disabled = true;
      btn.style.opacity = '0.3';
      btn.style.border = '1px solid var(--danger)';
    } else {
      btn.disabled = !active;
      btn.style.opacity = active ? '1' : '0.5';
      btn.style.border = '1px solid var(--border)';
    }
  });
  
  const actionControls = document.querySelectorAll('.grid-3 button');
  actionControls.forEach(btn => {
    const active = isController && (!isActionRunning || btn.onclick.toString().includes('sendMood') || btn.onclick.toString().includes('toggleAccessory'));
    btn.disabled = !active;
    btn.style.opacity = active ? '1' : '0.5';
  });
}

function openModal() { 
  if (session.role === 'pairing' || session.role === 'observer') {
    document.getElementById('loginModal').style.display = 'flex'; 
  }
}
function closeModal() { document.getElementById('loginModal').style.display = 'none'; }
function submitPin() {
  const pin = document.getElementById('pinInput').value.replace(/\s/g, '');
  closeModal();
  send({type: "pair", code: pin});
}

function connect() {
  session.ws = new WebSocket(`ws://${window.location.host}/ws`);
  session.ws.onopen = () => {
    session.connected = true;
    document.getElementById('wsBadge').textContent = 'Connected';
    document.getElementById('wsBadge').className = 'status-badge connected';
    logMsg('WebSocket connected');
    send({type: "hello"});
  };
  session.ws.onclose = () => {
    session.connected = false;
    document.getElementById('wsBadge').textContent = 'Disconnected';
    document.getElementById('wsBadge').className = 'status-badge disconnected';
    session.role = "disconnected";
    session.token = "";
    stopDriveLoop();
    updateRoleUI();
    logMsg('WebSocket disconnected', true);
    setTimeout(connect, 2000);
  };
  session.ws.onmessage = (e) => {
    try {
      const data = JSON.parse(e.data);
      if (data.type === 'telemetry') {
        session.telemetryRevision = data.revision;
        session.hasDrive = data.hasDrive;
        session.hasManipulators = data.hasManipulators;
        session.lastActionRunning = data.actionRunning;
        session.forwardMotionBlocked = data.forwardMotionBlocked;
        updateTelemetry(data);
        if (session.role === 'disconnected' || session.role === 'observer') {
          if (data.controllerPresent) session.role = 'busy';
          else if (data.pairingAvailable) session.role = 'pairing';
          else session.role = 'observer';
          updateRoleUI();
        }
      } else if (data.type === 'events') {
        data.events.forEach(ev => {
          logMsg(`[SYS] ${ev.sev}: ${ev.code}`, ev.sev === "WARN" || ev.sev === "ERROR", false);
        });
      } else if (data.type === 'ack') {
        // Handled silently
      } else if (data.type === 'paired') {
        session.token = data.token;
        session.role = 'controller';
        updateRoleUI();
        logMsg("Successfully paired and took control", false, true);
      } else if (data.type === 'error') {
        logMsg(`Error: ${data.code}`, true, false);
        if (data.code === 'controller_busy') {
          session.role = 'busy';
          updateRoleUI();
        } else if (data.code === 'pairing_failed') {
          logMsg("Pairing failed", true);
        } else if (data.code === 'not_controller' || data.code === 'bad_token' || data.code === 'superseded' || data.code === 'motors_locked') {
          stopDriveLoop();
        } else if (data.code === 'rate_limited') {
          // Disable pairing for 60s
          const btn = document.querySelector('#loginModal .btn');
          btn.disabled = true;
          let timeLeft = 60;
          btn.textContent = `Wait ${timeLeft}s`;
          const intv = setInterval(() => {
            timeLeft--;
            btn.textContent = `Wait ${timeLeft}s`;
            if (timeLeft <= 0) {
              clearInterval(intv);
              btn.disabled = false;
              btn.textContent = "Submit";
            }
          }, 1000);
        }
      }
    } catch(err) {}
  };
}

function send(obj) {
  if (session.ws && session.connected) {
    obj.v = 1;
    obj.id = session.nextRequestId++;
    if (session.token) obj.token = session.token;
    session.ws.send(JSON.stringify(obj));
  }
}

function updateTelemetry(t) {
  let rangeStr = t.rangeValid ? `${t.rangeMm} mm` : '-- mm';
  if (t.obstacleSafetyState) {
    rangeStr += ` (${t.obstacleSafetyState})`;
    if (t.obstacleSafetyState === 'blocked') {
      document.getElementById('rangeBadge').style.color = 'var(--danger)';
    } else if (t.obstacleSafetyState === 'caution') {
      document.getElementById('rangeBadge').style.color = 'orange';
    } else {
      document.getElementById('rangeBadge').style.color = 'var(--success)';
    }
  }
  document.getElementById('rangeBadge').textContent = rangeStr;
  document.getElementById('armedStatus').textContent = t.motorsArmed ? 'ARMED' : 'Disarmed';
  document.getElementById('armedStatus').style.color = t.motorsArmed ? 'var(--danger)' : 'var(--text-dim)';
  document.getElementById('motorPanel').className = t.motorsArmed ? 'panel armed' : 'panel disarmed';
  
  if (!t.hasDrive) {
    document.getElementById('motorPanel').style.display = 'none';
  } else {
    document.getElementById('motorPanel').style.display = 'block';
  }
  
  const actionPanel = document.querySelectorAll('.panel')[2]; // Actions panel is the 3rd panel
  if (actionPanel && actionPanel.querySelector('h2').textContent === 'Actions') {
    actionPanel.style.display = t.hasManipulators ? 'block' : 'none';
  }
  
  if (t.firmwareVersion) {
    document.getElementById('firmwareBadge').textContent = `${t.firmwareVersion} (${t.firmwareChannel})`;
  }

  const imuBadge = document.getElementById('imuBadge');
  if (!t.imuAvailable) {
    imuBadge.textContent = 'Unavailable';
    imuBadge.style.color = 'var(--danger)';
  } else if (!t.imuValid) {
    imuBadge.textContent = 'Waiting for data';
    imuBadge.style.color = 'orange';
  } else {
    imuBadge.textContent = 'Ready';
    imuBadge.style.color = 'var(--success)';
  }
  document.getElementById('accelBadge').textContent = t.imuValid
    ? `${t.accelG.x.toFixed(2)}, ${t.accelG.y.toFixed(2)}, ${t.accelG.z.toFixed(2)} g`
    : '--';
  document.getElementById('gyroBadge').textContent = t.imuValid
    ? `${t.gyroDps.x.toFixed(1)}, ${t.gyroDps.y.toFixed(1)}, ${t.gyroDps.z.toFixed(1)} °/s`
    : '--';
   
  updateRoleUI();
}

function sendStop() {
  stopDriveLoop();
  send({type: "stop"});
}

function toggleArm() {
  const willArm = document.getElementById('armedStatus').textContent === 'Disarmed';
  send({type: willArm ? "arm" : "disarm"});
}

function sendAction(actionStr) { send({type: "action", action: actionStr}); }
function sendMood(moodStr) { send({type: "mood", mood: moodStr}); }
function toggleAccessory(idx) {
  accState[idx] = !accState[idx];
  send({type: "accessory", index: idx, active: accState[idx]});
}

// Drive logic
function startDriveLoop(mode) {
  if (session.role !== 'controller') return;
  stopDriveLoop();
  session.heldDriveMode = mode;
  sendDriveCmd();
  session.activeDriveTimer = setInterval(sendDriveCmd, 150);
}

function sendDriveCmd() {
  if (!session.heldDriveMode) return;
  send({type: "move", mode: session.heldDriveMode, durationMs: 250});
}

function stopDriveLoop() {
  if (session.activeDriveTimer) {
    clearInterval(session.activeDriveTimer);
    session.activeDriveTimer = 0;
  }
  if (session.heldDriveMode) {
    session.heldDriveMode = "";
    send({type: "stop"});
  }
}

document.querySelectorAll('.dpad .btn').forEach(btn => {
  const mode = btn.dataset.mode;
  if (mode) {
    btn.onpointerdown = (e) => { 
      e.preventDefault(); 
      btn.setPointerCapture(e.pointerId);
      startDriveLoop(mode); 
    };
    btn.onpointerup = (e) => { e.preventDefault(); stopDriveLoop(); };
    btn.onpointercancel = (e) => { e.preventDefault(); stopDriveLoop(); };
    btn.addEventListener('lostpointercapture', () => { stopDriveLoop(); });
  }
});

window.addEventListener("keydown", (e) => {
  if (session.role !== "controller") return;
  if (["INPUT", "BUTTON"].includes(document.activeElement.tagName)) return;
  let mode = null;
  if (e.key === "ArrowUp" || e.key === "w" || e.key === "W") mode = "forward";
  else if (e.key === "ArrowDown" || e.key === "s" || e.key === "S") mode = "reverse";
  else if (e.key === "ArrowLeft" || e.key === "a" || e.key === "A") mode = "turn_left";
  else if (e.key === "ArrowRight" || e.key === "d" || e.key === "D") mode = "turn_right";
  
  if (mode) {
    e.preventDefault();
    if (session.heldDriveMode !== mode) startDriveLoop(mode);
  }
});

window.addEventListener("keyup", (e) => {
  if (session.role !== "controller") return;
  let mode = null;
  if (e.key === "ArrowUp" || e.key === "w" || e.key === "W") mode = "forward";
  else if (e.key === "ArrowDown" || e.key === "s" || e.key === "S") mode = "reverse";
  else if (e.key === "ArrowLeft" || e.key === "a" || e.key === "A") mode = "turn_left";
  else if (e.key === "ArrowRight" || e.key === "d" || e.key === "D") mode = "turn_right";
  
  if (mode && session.heldDriveMode === mode) {
    stopDriveLoop();
  }
});

// Safety: release control if browser loses focus or navigates away
document.addEventListener("visibilitychange", () => { if (document.hidden) sendStop(); });
window.addEventListener("blur", () => { sendStop(); });
window.addEventListener("pagehide", () => { sendStop(); });
window.addEventListener("beforeunload", () => { sendStop(); });

// Lease keepalive
setInterval(() => {
  if (session.role === 'controller' && session.connected) {
    send({type: "ping"});
  }
}, 5000);

connect();
</script>
</body>
</html>
)=====";
