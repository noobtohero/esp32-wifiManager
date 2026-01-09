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
