document.addEventListener("DOMContentLoaded", () => {
  // --- Escape Captive Portal Domain ---
  const h = window.location.hostname;
  if (
    h &&
    h !== "192.168.4.1" &&
    !h.endsWith(".local") &&
    (h.includes("msftconnecttest") ||
      h.includes("apple") ||
      h.includes("google"))
  ) {
    window.location.href = "http://192.168.4.1/";
    return;
  }

  const wifiList = document.getElementById("wifi-list");
  const ssidInput = document.getElementById("ssid");
  const foundNetworks = new Set();
  const networkData = new Map(); // Store network data for sorting

  // Clear placeholder when scan starts receiving data
  let firstReceive = true;

  // Auto-stop polling when no new networks found
  let lastNetworkCount = 0;
  let noChangeCount = 0;
  let pollInterval = null;
  let selectedSSID = null; // Track selected network

  // --- UI Helpers ---
  const scanningText = document.getElementById("scanning-text");

  // Refresh Button Logic
  document.getElementById("refresh-btn").onclick = (e) => {
    e.preventDefault();
    // Clear List
    wifiList.innerHTML =
      '<div style="text-align: center; padding: 1em; color: #888"><span id="scanning-text">Scanning...</span></div>';
    foundNetworks.clear();
    networkData.clear();
    firstReceive = true;
    lastNetworkCount = 0;
    noChangeCount = 0;
    selectedSSID = null;

    // Clear input fields
    ssidInput.value = "";
    document.getElementById("password").value = "";

    // Restart polling if stopped
    if (pollInterval) clearInterval(pollInterval);
    pollInterval = setInterval(fetchWifi, 1500);
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

      // Store network data
      networks.forEach((data) => {
        if (!foundNetworks.has(data.ssid)) {
          foundNetworks.add(data.ssid);
          networkData.set(data.ssid, data);
        }
      });

      // Sort networks by signal strength (strongest first)
      const sortedNetworks = Array.from(networkData.values()).sort(
        (a, b) => b.rssi - a.rssi
      );

      // Rebuild list with sorted networks
      if (sortedNetworks.length > 0 && firstReceive) {
        wifiList.innerHTML = "";
        firstReceive = false;
      }

      // Clear and rebuild list
      if (sortedNetworks.length > 0) {
        wifiList.innerHTML = "";
        sortedNetworks.forEach((data) => {
          const item = document.createElement("div");
          item.className = "wifi-item";

          // Add selected class if this is the selected network
          if (selectedSSID === data.ssid) {
            item.classList.add("selected");
          }

          // Determine Signal Icon/Text
          let signalColor = "#21ba45"; // Green
          let signalBars = "▂▄▆█"; // Full signal
          if (data.rssi < -70) {
            signalColor = "#fbbd08"; // Yellow
            signalBars = "▂▄▆";
          }
          if (data.rssi < -85) {
            signalColor = "#db2828"; // Red
            signalBars = "▂▄";
          }

          item.innerHTML = `
                        <div style="flex-grow: 1;">
                            <div style="font-weight: bold;">${data.ssid}</div>
                            <div style="font-size: 0.8em; color: #999;">
                              ${
                                data.secure === "true"
                                  ? "🔒 Secured"
                                  : "🔓 Open"
                              }
                            </div>
                        </div>
                        <div style="text-align: right; color:${signalColor}; font-weight: bold;">
                            <div style="font-size: 1.2em; letter-spacing: -2px;">${signalBars}</div>
                            <div style="font-size: 0.75em; margin-top: 2px;">${
                              data.rssi
                            } dBm</div>
                        </div>
                    `;

          item.onclick = () => {
            // Update selected state
            selectedSSID = data.ssid;
            document
              .querySelectorAll(".wifi-item")
              .forEach((el) => el.classList.remove("selected"));
            item.classList.add("selected");

            ssidInput.value = data.ssid;
            document.getElementById("password").focus();
          };

          wifiList.appendChild(item);
        });
      }

      // Auto-stop polling when no new networks found for 3 consecutive times
      const currentCount = foundNetworks.size;
      if (currentCount === lastNetworkCount && currentCount > 0) {
        noChangeCount++;
        if (noChangeCount >= 3) {
          console.log("No new networks found. Stopping scan.");
          clearInterval(pollInterval);
          pollInterval = null;
        }
      } else {
        noChangeCount = 0;
      }
      lastNetworkCount = currentCount;
    } catch (err) {
      console.error("Fetch error", err);
    }
  };

  fetchWifi();
  pollInterval = setInterval(fetchWifi, 1500);

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
