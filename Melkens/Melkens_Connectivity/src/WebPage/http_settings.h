#include "style.h"

String settings_html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <link rel="stylesheet" type="text/css" href="style.css">
  </head>
  <body>
    <div class="topnav">
      <h1>Settings</h1>
    </div>
    <div class="content">
      <div class="card-grid">
        <div class="card">
          <form action="/submit" method="POST">
            <p>
              <label for="ssid">SSID</label>
              <input type="text" id="ssid" name="ssid" value="%SSID%"><br>
              <label for="password">Password</label>
              <input type="text" id="pass" name="password" value="%PASSWORD%"><br>
              <label for="broker">Broker IP Address</label>
              <input type="text" id="broker" name="broker" value="%BROKER%"><br>
              <label for="espIp">ESP IP Address</label>
              <input type="text" id="espIp" name="espIp" value="%ESP_IP%"><br>
              <label for="gatewayIp">Gateway IP Address</label>
              <input type="text" id="gatewayIp" name="gatewayIp" value="%GATEWAY_IP%"><br>
              <input type="submit" value="Submit">
            </p>
          </form>
          <hr>
          <!-- OTA Update Section for ESP Firmware -->
          <h3 class="header_medium">Current ESP32 Firmware version: %FIRMWARE_VERSION%</h3>
          <form method="POST" action="/updateEsp" enctype="multipart/form-data">
            <input type="file" name="update"><br><br>
            <input type="submit" value="Flash">
          </form>
          <hr>
          <!-- OTA Update Section for Configuration -->
          <h3 class="header_medium">Configuration</h3>
          <form method="POST" action="/config" enctype="multipart/form-data">
            <input type="file" name="update"><br><br>
            <input type="submit" value="Flash">
          </form>
          <hr>
          <!-- OTA Update Section for PMB Firmware -->
          <h3 class="header_medium">PMB Firmware</h3>
          <form method="POST" action="/updatePmb" enctype="multipart/form-data">
            <input type="file" name="update"><br><br>
            <input type="submit" value="Flash">
          </form>
        </div>
      </div>
    </div>
  </body>
  </html>
)rawliteral" + style;
