#ifndef WM_WEB_ASSETS_H
#define WM_WEB_ASSETS_H

#include <Arduino.h>

const char WM_HTML_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ESP32 WiFi Setup</title>
    <style>
:root {
  --primary-color: #2185d0;
  --text-color: #333;
  --bg-color: #f4f7f6;
  --segment-bg: #fff;
  --border-color: rgba(34, 36, 38, 0.15);
}

body {
  background-color: var(--bg-color);
  font-family: "Lato", "Helvetica Neue", Arial, Helvetica, sans-serif;
  color: var(--text-color);
  margin: 0;
  padding: 20px;
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
}

.ui.container {
  width: 100%;
  max-width: 450px;
}

.ui.segment {
  background: var(--segment-bg);
  border-radius: 0.28571429rem;
  border: 1px solid var(--border-color);
  box-shadow: 0 1px 2px 0 rgba(34, 36, 38, 0.15);
  padding: 1.5em;
  margin-bottom: 1em;
}

.ui.header {
  border-bottom: 1px solid var(--border-color);
  margin-top: 0;
  margin-bottom: 1em;
  padding-bottom: 0.5em;
  font-size: 1.28571429rem;
  font-weight: 700;
  color: var(--primary-color);
}

.ui.list {
  margin: 1em 0;
  padding: 0;
  list-style: none;
}

.wifi-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0.8em;
  border-bottom: 1px solid #eee;
  cursor: pointer;
  transition: background 0.2s ease;
  animation: slideIn 0.3s ease-out;
}

.wifi-item:hover {
  background: #f9f9f9;
}

.wifi-item:last-child {
  border-bottom: none;
}

.ui.button {
  background-color: var(--primary-color);
  color: white;
  border: none;
  padding: 0.78571429em 1.5em;
  border-radius: 0.28571429rem;
  font-weight: 700;
  cursor: pointer;
  transition: background 0.2s ease, transform 0.1s ease;
  width: 100%;
}

.ui.button:hover {
  background-color: #1678c2;
  transform: translateY(-1px);
}

.ui.button:active {
  transform: translateY(0);
}

input[type="text"],
input[type="password"] {
  width: 100%;
  padding: 0.67857143em 1em;
  border: 1px solid var(--border-color);
  border-radius: 0.28571429rem;
  box-sizing: border-box;
  margin-bottom: 1em;
}

@keyframes slideIn {
  from {
    opacity: 0;
    transform: translateX(-10px);
  }
  to {
    opacity: 1;
    transform: translateX(0);
  }
}

.signal-strength {
  font-size: 0.9em;
  color: #888;
}

</style>
  </head>
  <body>
    <div class="ui container">
      <div class="ui segment">
        <h2 class="ui header">
          Select WiFi Network
          <button
            class="ui icon button basic mini"
            style="float: right"
            id="refresh-btn"
          >
            <!-- Simple SVG Icon for Refresh -->
            <svg viewBox="0 0 24 24" width="16" height="16" fill="currentColor">
              <path
                d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 1.01-.25 1.97-.7 2.8l1.46 1.46C19.54 15.03 20 13.57 20 12c0-4.42-3.58-8-8-8zm0 14c-3.31 0-6-2.69-6-6 0-1.01.25-1.97.7-2.8L5.24 7.74C4.46 8.97 4 10.43 4 12c0 4.42 3.58 8 8 8v3l4-4-4-4v3z"
              />
            </svg>
          </button>
        </h2>
        <p>Select a network or enter credentials manually.</p>

        <div id="wifi-list" class="ui list">
          <!-- WiFi items will appear here progressively -->
          <div style="text-align: center; padding: 1em; color: #888">
            <span id="scanning-text">Scanning...</span>
          </div>
        </div>

        <form action="/save" method="POST">
          <input
            type="text"
            name="ssid"
            id="ssid"
            placeholder="SSID"
            required
          />
          <input
            type="password"
            name="password"
            id="password"
            placeholder="Password"
          />
          <button type="submit" class="ui button">Connect</button>
        </form>
      </div>
    </div>
    <script>
document.addEventListener("DOMContentLoaded", () => {
  const wifiList = document.getElementById("wifi-list");
  const ssidInput = document.getElementById("ssid");
  const foundNetworks = new Set();

  // Clear placeholder when scan starts receiving data
  let firstReceive = true;

  // --- UI Helpers ---
  const scanningText = document.getElementById("scanning-text");

  // Refresh Button Logic
  document.getElementById("refresh-btn").onclick = (e) => {
    e.preventDefault();
    // Clear List
    wifiList.innerHTML =
      '<div style="text-align: center; padding: 1em; color: #888"><span id="scanning-text">Scanning...</span></div>';
    foundNetworks.clear();
    firstReceive = true;
  };

  let scanDots = 0;
  setInterval(() => {
    if (document.getElementById("scanning-text")) {
      scanDots = (scanDots + 1) % 4;
      document.getElementById("scanning-text").innerText =
        "Scanning" + ".".repeat(scanDots);
    }
  }, 500);

  // Use polling to receive WiFi scan results (Zero-Dependency)
  const fetchWifi = async () => {
    try {
      const response = await fetch("/list");
      const networks = await response.json();

      if (networks.length > 0 && firstReceive) {
        // Remove scanning text
        wifiList.innerHTML = "";
        firstReceive = false;
      }

      networks.forEach((data) => {
        if (!foundNetworks.has(data.ssid)) {
          foundNetworks.add(data.ssid);

          const item = document.createElement("div");
          item.className = "wifi-item";

          // Determine Signal Icon/Text (Simple simulation)
          let signalColor = "#21ba45"; // Green
          if (data.rssi < -70) signalColor = "#fbbd08"; // Yellow
          if (data.rssi < -85) signalColor = "#db2828"; // Red

          item.innerHTML = `
                        <div style="flex-grow: 1;">
                            <div style="font-weight: bold;">${data.ssid}</div>
                            <div style="font-size: 0.8em; color: #999;">
                              ${data.secure === "true" ? "🔒 Secured" : "Open"}
                            </div>
                        </div>
                        <div style="text-align: right; color:${signalColor}; font-weight: bold;">
                            ${data.rssi} dBm
                        </div>
                    `;

          item.onclick = () => {
            ssidInput.value = data.ssid;
            document.getElementById("password").focus();
          };

          wifiList.appendChild(item);
        }
      });
    } catch (err) {
      console.error("Fetch error", err);
    }
  };

  fetchWifi();
  const pollInterval = setInterval(fetchWifi, 1500);

  // --- Confirmation UI Refactor ---
  const form = document.querySelector("form");
  form.onsubmit = async (e) => {
    e.preventDefault();
    const btn = form.querySelector("button");
    const originalText = btn.innerHTML;

    btn.disabled = true;
    btn.innerHTML = "Verifying Credentials..."; // Change text to indicate testing

    const formData = new FormData(form);
    const params = new URLSearchParams();
    for (const pair of formData) {
      params.append(pair[0], pair[1]);
    }

    try {
      const response = await fetch("/save", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: params,
      });

      const result = await response.json(); // Expect JSON now

      if (result.status === "connected") {
        btn.style.backgroundColor = "#21ba45"; // Green
        btn.innerHTML = "Success! Restarting...";
        // Show Success UI
        document.querySelector(".ui.segment").innerHTML = `
           <h2 class="ui header" style="color: #21ba45">Connected!</h2>
           <p>Device is restarting to connect to <b style="color:#2185d0">${params.get(
             "ssid"
           )}</b>.</p>
           <p>Please reconnect your phone to your home WiFi.</p>
           <div class="ui active centered inline loader"></div>
        `;
      } else {
        throw new Error("Auth Failed");
      }
    } catch (err) {
      // Handle Failure (Wrong Password)
      btn.disabled = false;
      btn.style.backgroundColor = "#db2828"; // Red
      btn.innerHTML = "Failed! Check Password";

      // Auto-revert button after 3s
      setTimeout(() => {
        btn.style.backgroundColor = "";
        btn.innerHTML = originalText;
      }, 3000);

      // Show error message
      alert("Connection Failed! Please check your password and try again.");
    }
  };
});

</script>
  </body>
</html>

)rawliteral";

#endif
