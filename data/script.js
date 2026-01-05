document.addEventListener("DOMContentLoaded", () => {
  const wifiList = document.getElementById("wifi-list");
  const ssidInput = document.getElementById("ssid");
  const foundNetworks = new Set();

  // Clear placeholder when scan starts receiving data
  let firstReceive = true;

  // Use Server-Sent Events to receive WiFi scan results progressively
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

  source.addEventListener("error", (e) => {
    console.error("SSE connection error", e);
  });

  // --- Confirmation UI Refactor ---
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
        btn.style.backgroundColor = "#21ba45"; // Fomantic Green
        btn.innerHTML = "Saved! Restarting...";
        // Show a friendly message
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
      btn.style.backgroundColor = "#db2828"; // Fomantic Red
      btn.innerHTML = "Error! Try Again";
      setTimeout(() => {
        btn.style.backgroundColor = "";
        btn.innerHTML = originalText;
      }, 3000);
    }
  };
});
