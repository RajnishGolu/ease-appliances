#!/usr/bin/env python3
import os
import xml.etree.ElementTree as ET

ARTBOARDS_DIR = "/Users/rajnishmishra/.gemini/antigravity/scratch/my-smart-plug/figma/artboards"
OUTPUT_FILE = "/Users/rajnishmishra/.gemini/antigravity/scratch/my-smart-plug/figma/master_flow_canvas.svg"

# Grid configuration: 5 columns x 3 rows
# Screen width: 390, height: 844
COL_GAP = 120
ROW_GAP = 160
PADDING = 100
SCREEN_W = 390
SCREEN_H = 844

# Layout mapping: (filename, col, row, title, subtitle)
FLOW_GRID = [
    # Row 0: Onboarding & Discovery
    ("01_splash_screen.svg", 0, 0, "1. Splash Screen", "Auto-transition or tap"),
    ("02_home_my_devices.svg", 1, 0, "2. Home / My Devices", "Device list & quick switch"),
    ("03_add_smart_plug.svg", 2, 0, "3. Add Smart Plug", "Setup initiation"),
    ("04_finding_smart_plug.svg", 3, 0, "4. Finding Smart Plug", "Radar discovery"),
    ("05_wifi_setup.svg", 4, 0, "5. Wi-Fi Setup", "Network selection & password"),
    
    # Row 1: Connection & Main Controls
    ("06_setting_up_progress.svg", 0, 1, "6. Setting Up", "4-stage animated progress"),
    ("06b_connection_failed.svg", 1, 1, "6b. Connection Failed", "Error state & retry"),
    ("07_smart_plug_ready.svg", 2, 1, "7. Smart Plug Ready", "Activation success"),
    ("08_device_control_on.svg", 3, 1, "8. Device Control (ON)", "Tactile circular dial"),
    ("08b_device_control_off.svg", 4, 1, "8b. Device Control (OFF)", "Power off state"),
    
    # Row 2: Management & Edge States
    ("09_device_settings.svg", 0, 2, "9. Device Settings", "Settings & danger action"),
    ("10_wifi_settings.svg", 1, 2, "10. Wi-Fi Settings", "Network reconfiguration"),
    ("11_offline_state.svg", 2, 2, "11. Offline State", "Disconnected handling"),
    ("12_empty_home_state.svg", 3, 2, "12. Empty Home State", "Zero-device onboarding")
]

TOTAL_COLS = 5
TOTAL_ROWS = 3
CANVAS_W = PADDING * 2 + TOTAL_COLS * SCREEN_W + (TOTAL_COLS - 1) * COL_GAP
CANVAS_H = PADDING * 2 + TOTAL_ROWS * SCREEN_H + (TOTAL_ROWS - 1) * ROW_GAP + 120 # extra for title header

def get_screen_coords(col, row):
    x = PADDING + col * (SCREEN_W + COL_GAP)
    y = PADDING + 120 + row * (SCREEN_H + ROW_GAP)
    return x, y

def read_svg_content(filename):
    path = os.path.join(ARTBOARDS_DIR, filename)
    with open(path, "r") as f:
        content = f.read()
    # Extract inner content between <svg ...> and </svg>
    start = content.find(">") + 1
    end = content.rfind("</svg>")
    return content[start:end]

svg_parts = []
svg_parts.append(f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {CANVAS_W} {CANVAS_H}" width="{CANVAS_W}" height="{CANVAS_H}">
  <defs>
    <filter id="masterCardShadow" x="-10%" y="-10%" width="120%" height="120%">
      <feDropShadow dx="0" dy="24" stdDeviation="28" flood-color="#0F172A" flood-opacity="0.1" />
      <feDropShadow dx="0" dy="8" stdDeviation="12" flood-color="#0F172A" flood-opacity="0.05" />
    </filter>
    <marker id="arrow" viewBox="0 0 10 10" refX="6" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
      <path d="M 0 1 L 10 5 L 0 9 z" fill="#6366F1"/>
    </marker>
    <marker id="arrowRed" viewBox="0 0 10 10" refX="6" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
      <path d="M 0 1 L 10 5 L 0 9 z" fill="#EF4444"/>
    </marker>
  </defs>

  <!-- Master Canvas Background -->
  <rect width="{CANVAS_W}" height="{CANVAS_H}" fill="#0F172A" />
  
  <!-- Canvas Header -->
  <g id="Master_Header" transform="translate({PADDING}, 60)">
    <text x="0" y="32" font-family="-apple-system, BlinkMacSystemFont, 'Plus Jakarta Sans', sans-serif" font-size="34" font-weight="900" fill="#FFFFFF" letter-spacing="-0.5">My Smart Plug — Complete Figma Prototype Flow</text>
    <text x="0" y="62" font-family="-apple-system, BlinkMacSystemFont, 'Plus Jakarta Sans', sans-serif" font-size="16" font-weight="500" fill="#94A3B8">High-Fidelity Mobile Design System (390 × 844 px) • 12 Screens • Consumer Terminology • Reusable Components</text>
  </g>
""")

# Place all screens
for filename, col, row, title, subtitle in FLOW_GRID:
    x, y = get_screen_coords(col, row)
    inner_svg = read_svg_content(filename)
    
    # Card container with title label above
    screen_group = f"""
    <!-- Screen: {title} -->
    <g id="Frame_{filename.replace('.svg', '')}" transform="translate({x}, {y})">
      <!-- Title Label above screen -->
      <text x="0" y="-36" font-family="-apple-system, BlinkMacSystemFont, 'Plus Jakarta Sans', sans-serif" font-size="16" font-weight="800" fill="#F8FAFC">{title}</text>
      <text x="0" y="-16" font-family="-apple-system, BlinkMacSystemFont, 'Plus Jakarta Sans', sans-serif" font-size="12" font-weight="500" fill="#818CF8">{subtitle}</text>

      <!-- Frame Shell -->
      <g filter="url(#masterCardShadow)">
        <rect width="{SCREEN_W}" height="{SCREEN_H}" rx="40" fill="#FFFFFF" />
        <g clip-path="url(#clip_{col}_{row})">
          {inner_svg}
        </g>
        <!-- Device Frame Border -->
        <rect width="{SCREEN_W}" height="{SCREEN_H}" rx="40" fill="none" stroke="#334155" stroke-width="6" />
      </g>
    </g>
    """
    svg_parts.append(screen_group)

# Add connector flow lines between screens
def add_arrow(x1, y1, x2, y2, label="", color="#6366F1", marker="url(#arrow)"):
    mid_x = (x1 + x2) / 2
    mid_y = (y1 + y2) / 2
    return f"""
    <g class="flow-connector">
      <path d="M {x1} {y1} L {x2} {y2}" fill="none" stroke="{color}" stroke-width="2.5" stroke-dasharray="6 4" marker-end="{marker}"/>
      {f'<rect x="{mid_x - 50}" y="{mid_y - 14}" width="100" height="24" rx="12" fill="#1E293B" stroke="{color}" stroke-width="1"/><text x="{mid_x}" y="{mid_y + 2}" font-family="sans-serif" font-size="10" font-weight="700" fill="#FFFFFF" text-anchor="middle">{label}</text>' if label else ''}
    </g>
    """

arrows = []
# Row 0 arrows: Splash -> Home -> Add -> Finding -> Wifi Setup
x0_0, y0_0 = get_screen_coords(0, 0)
x1_0, y1_0 = get_screen_coords(1, 0)
x2_0, y2_0 = get_screen_coords(2, 0)
x3_0, y3_0 = get_screen_coords(3, 0)
x4_0, y4_0 = get_screen_coords(4, 0)

arrows.append(add_arrow(x0_0 + SCREEN_W + 10, y0_0 + 400, x1_0 - 15, y1_0 + 400, "Auto (2.5s)"))
arrows.append(add_arrow(x1_0 + SCREEN_W + 10, y1_0 + 740, x2_0 - 15, y2_0 + 740, "+ Add Plug"))
arrows.append(add_arrow(x2_0 + SCREEN_W + 10, y2_0 + 720, x3_0 - 15, y3_0 + 720, "Start Setup"))
arrows.append(add_arrow(x3_0 + SCREEN_W + 10, y3_0 + 750, x4_0 - 15, y4_0 + 750, "Continue"))

# Row 1 arrows: Wifi Setup -> Setting Up -> Ready -> Control ON <-> Control OFF
x0_1, y0_1 = get_screen_coords(0, 1)
x1_1, y1_1 = get_screen_coords(1, 1)
x2_1, y2_1 = get_screen_coords(2, 1)
x3_1, y3_1 = get_screen_coords(3, 1)
x4_1, y4_1 = get_screen_coords(4, 1)

arrows.append(add_arrow(x4_0 + SCREEN_W/2, y4_0 + SCREEN_H + 10, x0_1 + SCREEN_W/2, y0_1 - 40, "Connect Wi-Fi"))
arrows.append(add_arrow(x0_1 + SCREEN_W + 10, y0_1 + 400, x2_1 - 15, y2_1 + 400, "Success Path"))
arrows.append(add_arrow(x0_1 + SCREEN_W + 10, y0_1 + 500, x1_1 - 15, y1_1 + 500, "Fail Path", color="#EF4444", marker="url(#arrowRed)"))
arrows.append(add_arrow(x2_1 + SCREEN_W + 10, y2_1 + 680, x3_1 - 15, y3_1 + 680, "Start Using"))
arrows.append(add_arrow(x3_1 + SCREEN_W + 10, y3_1 + 340, x4_1 - 15, y4_1 + 340, "Tap Dial"))
arrows.append(add_arrow(x4_1 - 15, y4_1 + 380, x3_1 + SCREEN_W + 10, y3_1 + 380, "Tap Dial"))

svg_parts.extend(arrows)
svg_parts.append("</svg>")

with open(OUTPUT_FILE, "w") as f:
    f.write("\n".join(svg_parts))

print(f"Generated {OUTPUT_FILE} ({os.path.getsize(OUTPUT_FILE)} bytes) with dimensions {CANVAS_W}x{CANVAS_H}!")
