# My Smart Plug — Mobile Application Prototype

A high-fidelity, interactive mobile smart-home application prototype designed for Wi-Fi smart plugs. Built strictly from a consumer-first perspective with modern mobile UX standards, subtle micro-interactions, and **zero technical jargon** (no MQTT, ESP8266, GPIO, brokers, or provisioning).

---

## Key Highlights

- **Mobile-First Design Target**: Calibrated for standard modern smartphone viewports (**390 × 844 px**), encased in an iPhone 15 style device frame with dynamic island and status bar, while being fully responsive on mobile viewports.
- **Modern Premium Aesthetic**: Clean white/light surfaces (`#F8FAFC`, `#FFFFFF`), rounded cards and buttons (`rounded-2xl`, `rounded-3xl`), and subtle indigo/violet accents (`#6366F1` to `#4F46E5`).
- **Tactile Power Controls**:
  - Compact power switch on device cards with immediate visual feedback.
  - Large 180px circular tactile power button on Device Control with radial pulse glow when ON and smooth transition when OFF.
  - Synthesized Web Audio API tactile switch sounds for realistic physical sensation.
- **Consumer Terminology**:
  - *Smart Plug* (not relay or module)
  - *Power* (not GPIO pin state)
  - *Wi-Fi* (not SSID / 802.11 b/g/n)
  - *Connected / Offline* (not MQTT broker keep-alive status)
  - *Setup* (not provisioning)
- **Interactive Review Toolbar**: Built-in presentation bar at the top with a screen jumper (1 to 12), state toggler (Empty vs. Active), audio toggle, and demo resets.

---

## 12 Screen & State Map

| # | Screen | Description & Primary Interactions |
|---|---|---|
| 1 | **Splash Screen** | Glowing smart plug icon, "My Smart Plug", "Control your home, from anywhere." Auto-transitions to Home after 2.5s or tap anywhere to skip. |
| 2 | **Home / My Devices** | Header with "Good evening", "My Smart Plugs", device card with name, live green "Connected" badge, Wi-Fi strength, toggleable power switch, last updated time, and "+ Add Smart Plug" button. Tap card to open Device Control. |
| 3 | **Add Smart Plug** | "Add your Smart Plug", vector illustration, step 1 of 3, "Start Setup" primary action, "Already configured? Connect existing plug" secondary link. |
| 4 | **Finding Smart Plug** | Step 2 of 3. Continuous animated radar pulses radiating from smart plug glyph, "Looking for your Smart Plug", "Searching nearby...". Animates to "Smart Plug found" with "Smart Plug • SP-001" card and "Continue" button. |
| 5 | **Wi-Fi Setup** | Step 3 of 3. List of available Wi-Fi networks (`Home Wi-Fi`, `Rajnish Home`, `Office Wi-Fi`, `Other Network`), refresh networks button, password input with show/hide eye toggle, validation against empty inputs, and "Remember this network" checkbox. Includes demo toggle to test connection error. |
| 6 | **Setting Up (Progress)** | 4-stage sequential progress stepper (✓ Smart Plug found → ✓ Wi-Fi configured → ● Connecting to your home network → ○ Getting ready) with animated progress bar. |
| 6b | **Connection Failed (Error)** | Friendly error state: "Couldn't connect", "Check your Wi-Fi password and try again", troubleshooting tips, "Try Again" and "Change Wi-Fi" buttons. |
| 7 | **Smart Plug Ready** | Animated green checkmark badge, "You're all set!", "Your Smart Plug is connected and ready to use", device preview card, "Start Using" button, "Rename Smart Plug" button. |
| 8 | **Device Control** | Full screen control for "Living Room". Large tactile circular ON/OFF switch (glowing purple radial glow when ON, cool slate when OFF). Status badge, Wi-Fi connection info, Rename modal trigger, Wi-Fi settings trigger, and "Go Offline" simulation trigger. |
| 9 | **Device Settings** | "Smart Plug Settings" with options: Smart Plug Name (tap to rename), Wi-Fi, Connection Status, Firmware (v2.4.1), and soft red "Remove Smart Plug" danger button with confirmation modal. |
| 10 | **Wi-Fi Settings** | Shows current status "Connected", current network "Home Wi-Fi", signal strength, and "Change Wi-Fi" button leading to Wi-Fi Setup. |
| 11 | **Offline State** | Amber warning banner: "Smart Plug Offline • We can't reach your Smart Plug right now." Circular power control disabled with muted styling. "Try Again" reconnect action and "Reconnect Wi-Fi" button. |
| 12 | **Empty Home State** | Displayed when no plugs are registered. Friendly illustration, "No Smart Plugs yet", "Add your first Smart Plug to get started", and "+ Add Smart Plug" action. |

---

## Reusable Component Library

- **Buttons**:
  - `PrimaryButton`: Violet-indigo gradient with soft elevation shadow and active press scale.
  - `SecondaryButton`: Light slate fill with crisp border.
  - `DangerButton`: Soft rose background with red text.
- **Power Switches**:
  - Card Switch: Compact pill switch with smooth thumb transition.
  - Circular Dial: 180px tactile button with ambient radial glow.
- **Cards & Rows**:
  - `DeviceCard`: Rounded-3xl card with responsive power switch.
  - `WifiNetworkRow`: Selectable list item with signal strength, lock icon, and radio checkmark.
- **Modals**:
  - Bottom-sheet modal for **Rename Smart Plug** with auto-focus input.
  - Warning modal for **Remove Smart Plug** with confirmation safeguard.
- **Toast Notifications**:
  - Animated floating feedback pills ("Power turned on", "Renamed to Living Room", "Networks refreshed").

---

## How to Run the Prototype

### Option 1: Direct File Open
Open `index.html` directly in any modern web browser (Google Chrome, Safari, Firefox, or Edge). No build step required!

```bash
open /Users/rajnishmishra/.gemini/antigravity/scratch/my-smart-plug/index.html
```

### Option 2: Local Web Server
You can run a lightweight development server via Node:

```bash
cd /Users/rajnishmishra/.gemini/antigravity/scratch/my-smart-plug
npx serve -l 3000 .
```
Then visit `http://localhost:3000` in your browser.
