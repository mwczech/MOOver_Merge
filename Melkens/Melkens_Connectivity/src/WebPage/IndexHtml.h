char html[] = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <meta charset="UTF-8">
  <title>Joystick WebSocket</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 30px;
      user-select: none;
    }

    canvas {
      border: 1px solid #000;
      touch-action: none;
    }

    .bit {
      width: 20px;
      height: 20px;
      border-radius: 4px;
      background-color: #bbb;
      border: 1px solid #666;
      transition: background-color 0.2s;
    }

    .bit.on {
      background-color: #4caf50;
    }
  </style>
</head>

<body>
  <div id="wsStatus" style="color: red; text-align: center; font-weight: bold; display: none;">
    WebSocket disconnected. Please refresh the page.
  </div>
  <h1>Motion Controller</h1>
  <canvas id="joystick" width="300" height="300"></canvas>
  <p>X: <span id="xVal">0</span></p>
  <p>Y: <span id="yVal">0</span></p>

  <div class="slider-container">
    <label for="speedSlider">Auger Speed:</label>
    <input type="range" id="augerSpeedSlider" min="0" max="1500" value="50" oninput="updateSpeedValue(this.value)">
    <!--<span id="speedValue">50</span> -->
  </div>

  <div id="route" style="text-align: center; margin-top: 30px;">
    <h3>Select Route:</h3>

    <form id="routeOptions"
      style="display: flex; justify-content: center; gap: 10px; flex-wrap: wrap; margin-bottom: 20px;">
      <label><input type="radio" name="routeOption" value=0> A</label>
      <label><input type="radio" name="routeOption" value=1> B</label>
      <label><input type="radio" name="routeOption" value=2> C</label>
      <label><input type="radio" name="routeOption" value=3> D</label>
      <label><input type="radio" name="routeOption" value=4> F</label>
      <label><input type="radio" name="routeOption" value=5> G</label>
      <label><input type="radio" name="routeOption" value=6> H</label>
      <label><input type="radio" name="routeOption" value=7> I</label>
      <label><input type="radio" name="routeOption" value=8> J</label>
      <label><input type="radio" name="routeOption" value=9> K</label>
    </form>

    <div class="buttons">
      <button id="playButton">▶️ Play</button>
      <button id="pauseButton">⏸️ Pause</button>
      <button id="stopButton">⏹️ Stop</button>
    </div>
  </div>

  <h3>Settings:</h3>
  <div class="toggle-container">
    <input type="checkbox" id="powerCheckbox" name="vehicle1" value="on">
    <label for="vehicle1">Power</label><br>
    <input type="checkbox" id="chargingCheckbox" name="vehicle1" value="on">
    <label for="vehicle1">Charging</label><br>
  </div>

  <div style="max-width: 400px; margin: auto; text-align: left; font-family: sans-serif;">
    <h3>Status:</h3>
    <p>PMB Connection: <span id="pmbConnection">--</span></p>
    <p>Motor Right Speed: <span id="motorRightSpeed">--</span> RPM</p>
    <p>Motor Left Speed: <span id="motorLeftSpeed">--</span> RPM</p>
    <p>Battery Voltage: <span id="batteryVoltage">--</span> mV</p>
    <p>ADC Current: <span id="adcCurrent">--</span> mA</p>
    <p>Thumble Current: <span id="thumbleCurrent">--</span> mA</p>
    <p>CRC IMU → PMB Errors: <span id="crcImu2PmbErrorCount">--</span></p>
    <p>CRC PMB → IMU Errors: <span id="crcPmb2ImuErrorCount">--</span></p>
    <p>CRC ESP → IMU Errors: <span id="crcEsp2ImuErrorCount">--</span></p>
  </div>

  <h3>Magnet Bar:</h3>
  <div id="bitIndicatorContainer" style="text-align: center; margin-top: 20px;">
    <div id="bitLabels" style="display: flex; gap: 6px; justify-content: center; font-size: 14px; margin-bottom: 5px;">
    </div>
    <div id="bitIndicator" style="display: flex; gap: 4px; justify-content: center;"></div>
  </div>

  <script>
    const canvas = document.getElementById("joystick");
    const ctx = canvas.getContext("2d");
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const radius = 100;
    const handleRadius = 20;
    let currentX = centerX;
    let currentY = centerY;
    let dragging = false;

    // WebSocket using non secure connection
    const gateway = `ws://${window.location.hostname}/ws`;
    const websocket = new WebSocket(gateway);

    websocket.onopen = () => {
      console.log("WebSocket connection opened");
      document.getElementById("wsStatus").style.display = "none";
    };

    websocket.onclose = () => {
      console.warn("WebSocket disconnected");
      document.getElementById("wsStatus").style.display = "block";
    };

    websocket.onerror = (error) => {
      console.error("WebSocket error:", error);
      document.getElementById("wsStatus").style.display = "block";
    };

    websocket.onmessage = function (event) {
      try {
        const data = JSON.parse(event.data);
        updateBitIndicators(data.magnetBarStatus);
        document.getElementById("pmbConnection").innerText = data.pmbConnection;
        document.getElementById("motorRightSpeed").innerText = data.motorRightSpeed;
        document.getElementById("motorLeftSpeed").innerText = data.motorLeftSpeed;
        document.getElementById("batteryVoltage").innerText = data.batteryVoltage;
        document.getElementById("adcCurrent").innerText = data.adcCurrent;
        document.getElementById("thumbleCurrent").innerText = data.thumbleCurrent;
        document.getElementById("crcImu2PmbErrorCount").innerText = data.crcImu2PmbErrorCount;
        document.getElementById("crcPmb2ImuErrorCount").innerText = data.crcPmb2ImuErrorCount;
        document.getElementById("crcEsp2ImuErrorCount").innerText = data.crcEsp2ImuErrorCount;
      } catch (e) {
        console.error("Error parsing JSON data:", e);
      }
    };

    function sendJoystickData(x, y) {
      const payload = JSON.stringify({ type: "joystick", x: x, y: y });
      if (websocket.readyState === WebSocket.OPEN) {
        websocket.send(payload);
      }
    }

    function sendAuger(augerSpeed) {
      const payload = JSON.stringify({ type: "auger", value: parseInt(augerSpeed) });
      if (websocket.readyState === WebSocket.OPEN) {
        websocket.send(payload);
      }
    }

    function sendRoute(route) {
      const payload = JSON.stringify({ type: "route", value: route });
      if (websocket.readyState === WebSocket.OPEN) {
        websocket.send(payload);
      }
    }

    function sendButton(buttonName) {
      const payload = JSON.stringify({ type: "button", value: buttonName });
      if (websocket.readyState === WebSocket.OPEN) {
        websocket.send(payload);
      }
    }

    function sendCheckboxState(id, checked) {
      const payload = JSON.stringify({ type: "checkbox", id: id, value: checked });
      if (websocket.readyState === WebSocket.OPEN) {
        websocket.send(payload);
      }
    }

    function drawJoystick(x, y) {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.beginPath();
      ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
      ctx.stroke();
      ctx.beginPath();
      ctx.arc(x, y, handleRadius, 0, Math.PI * 2);
      ctx.fillStyle = "blue";
      ctx.fill();
    }

    function handleMove(e) {
      const rect = canvas.getBoundingClientRect();
      const clientX = e.touches ? e.touches[0].clientX : e.clientX;
      const clientY = e.touches ? e.touches[0].clientY : e.clientY;
      const x = clientX - rect.left;
      const y = clientY - rect.top;

      const dx = x - centerX;
      const dy = y - centerY;
      const distance = Math.min(Math.sqrt(dx * dx + dy * dy), radius);

      const angle = Math.atan2(dy, dx);
      currentX = centerX + distance * Math.cos(angle);
      currentY = centerY + distance * Math.sin(angle);

      const normX = Math.round(((currentX - centerX) / radius) * 100);
      const normY = Math.round(((centerY - currentY) / radius) * 100); // Y reversed

      document.getElementById("xVal").textContent = normX;
      document.getElementById("yVal").textContent = normY;
      drawJoystick(currentX, currentY);
      sendJoystickData(normX, normY);
    }

    function resetJoystick() {
      currentX = centerX;
      currentY = centerY;
      drawJoystick(currentX, currentY);
      document.getElementById("xVal").textContent = 0;
      document.getElementById("yVal").textContent = 0;
      sendJoystickData(0, 0);
    }

    canvas.addEventListener("mousedown", () => dragging = true);
    canvas.addEventListener("mouseup", () => { dragging = false; resetJoystick(); });
    canvas.addEventListener("mouseleave", () => { dragging = false; resetJoystick(); });
    canvas.addEventListener("mousemove", (e) => { if (dragging) handleMove(e); });

    canvas.addEventListener("touchstart", (e) => { dragging = true; handleMove(e); });
    canvas.addEventListener("touchend", () => { dragging = false; resetJoystick(); });
    canvas.addEventListener("touchcancel", () => { dragging = false; resetJoystick(); });
    canvas.addEventListener("touchmove", (e) => { if (dragging) handleMove(e); });

    drawJoystick(currentX, currentY);

    document.getElementById("augerSpeedSlider").addEventListener("input", (e) => {
      sendAuger(e.target.value);
    });

    document.querySelectorAll("input[name='routeOption']").forEach(el => {
      el.addEventListener("change", () => sendRoute(el.value));
    });

    document.getElementById("playButton").addEventListener("click", () => sendButton(2));
    document.getElementById("pauseButton").addEventListener("click", () => sendButton(1));
    document.getElementById("stopButton").addEventListener("click", () => sendButton(0));
    document.getElementById("powerCheckbox").addEventListener("change", function () { sendCheckboxState("power", this.checked); });
    document.getElementById("chargingCheckbox").addEventListener("change", function () { sendCheckboxState("charging", this.checked); });

    const bitLabels = document.getElementById("bitLabels");
    const bitIndicator = document.getElementById("bitIndicator");

    for (let i = 31; i >= 0; i--) {
      // Labels
      let label = document.createElement("div");
      label.style.width = "20px";
      if (i > 16) label.innerText = "L" + (i - 16);
      else if (i === 16) label.innerText = "0";
      else label.innerText = "P" + (15 - i);
      bitLabels.appendChild(label);

      // Indicators
      let bit = document.createElement("div");
      bit.className = "bit";
      bit.id = "bit" + i;
      bit.title = "Bit " + i;
      bit.style.width = "20px";
      bit.style.height = "20px";
      bit.style.borderRadius = "4px";
      bit.style.border = "1px solid #ccc";
      bit.style.backgroundColor = "#eee";
      bitIndicator.appendChild(bit);
    }

    function updateBitIndicators(value) {
      for (let i = 0; i < 32; i++) {
        const bit = (value >> i) & 1;
        const bitElement = document.getElementById("bit" + i);
        if (bitElement) {
          bitElement.style.backgroundColor = bit ? "#4caf50" : "#eee";
        }
      }
    }
  </script>
</body>
</html>
)rawliteral";
