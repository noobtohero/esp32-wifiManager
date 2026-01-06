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
/* style.css content embedded here for simpler serving */
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
        <h2 class="ui header">WiFi Configuration</h2>
        <p>Select a network or enter credentials manually.</p>

        <div id="wifi-list" class="ui list">
          <div style="text-align: center; padding: 1em; color: #888">
            Scanning for networks...
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
/* script.js content embedded here */
document.addEventListener("DOMContentLoaded", () => {
  const wifiList = document.getElementById("wifi-list");
  const ssidInput = document.getElementById("ssid");
  const foundNetworks = new Set();

  let firstReceive = true;
  const source = new EventSource("/events");

  source.addEventListener("wifi-found", (e) => {
    if (firstReceive) {
      wifiList.innerHTML = "";
      firstReceive = false;
    }

    const data = JSON.parse(e.data);
    if (!foundNetworks.has(data.ssid)) {
      foundNetworks.add(data.ssid);

      const item = document.createElement("div");
      item.className = "wifi-item";
      item.innerHTML = `
                <div>
                    <span>${data.ssid}</span>
                    <span style="font-size: 0.8em; color: #999;">${
                      data.secure ? "🔒" : ""
                    }</span>
                </div>
                <span class="signal-strength">${data.rssi} dBm</span>
            `;

      item.onclick = () => {
        ssidInput.value = data.ssid;
        document.getElementById("password").focus();
      };

      wifiList.appendChild(item);
    }
  });

  const form = document.querySelector("form");
  form.onsubmit = async (e) => {
    e.preventDefault();
    const btn = form.querySelector("button");
    const originalText = btn.innerHTML;

    btn.disabled = true;
    btn.innerHTML = "Connecting...";

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

      if (response.ok) {
        btn.style.backgroundColor = "#21ba45";
        btn.innerHTML = "Saved! Restarting...";
        document.querySelector(".ui.segment").innerHTML = `
                    <h2 class="ui header" style="color: #21ba45">Success!</h2>
                    <p>Device is restarting to connect to <b>${params.get(
                      "ssid"
                    )}</b>.</p>
                    <p>Wait about 10 seconds, then reconnect your phone to your home WiFi.</p>
                `;
      } else {
        throw new Error("Save failed");
      }
    } catch (err) {
      btn.disabled = false;
      btn.style.backgroundColor = "#db2828";
      btn.innerHTML = "Error! Try Again";
      setTimeout(() => {
        btn.style.backgroundColor = "";
        btn.innerHTML = originalText;
      }, 3000);
    }
  };
});
    </script>
  </body>
</html>
)rawliteral";

#endif
