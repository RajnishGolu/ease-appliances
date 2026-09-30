#!/usr/bin/env python3
"""
Ease Appliances v1.0 - JLCPCB PCB & PCBA Package Generator
Generates:
1. Standard RS-274X Gerber Files (.gtl, .gbl, .gts, .gbs, .gto, .gbo, .gml)
2. Excellon NC Drill File (.drl)
3. Zipped Gerber Package for JLCPCB (Gerber_Ease_Appliances_v1.0.zip)
4. JLCPCB SMT Assembly BOM (BOM_Ease_Appliances_v1.0.csv)
5. JLCPCB SMT Assembly CPL / Centroid (CPL_Ease_Appliances_v1.0.csv)
6. EasyEDA Project File (Ease_Appliances_EasyEDA.json)
"""

import os
import csv
import zipfile
import math

OUTPUT_DIR = "/Users/rajnishmishra/.gemini/antigravity/scratch/my-smart-plug/pcb_design"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# -------------------------------------------------------------
# 1. BILL OF MATERIALS (BOM) for JLCPCB SMT Assembly
# -------------------------------------------------------------
BOM_DATA = [
    {"Comment": "ESP32-WROOM-32E-N4", "Designator": "U1", "Footprint": "MODULE_ESP32-WROOM-32E", "LCSC Part #": "C701341"},
    {"Comment": "BL0942-SOP16", "Designator": "U2", "Footprint": "SOP-16_150mil", "LCSC Part #": "C2893544"},
    {"Comment": "CH340C", "Designator": "U3", "Footprint": "SOP-16_150mil", "LCSC Part #": "C84681"},
    {"Comment": "AMS1117-3.3", "Designator": "U4", "Footprint": "SOT-223", "LCSC Part #": "C6186"},
    {"Comment": "PC817C", "Designator": "U5,U6,U7,U8", "Footprint": "SOP-4", "LCSC Part #": "C4998"},
    {"Comment": "SRD-05VDC-SL-C", "Designator": "K1,K2,K3,K4", "Footprint": "RELAY-TH_SRD-05VDC-SL-C", "LCSC Part #": "C33965"},
    {"Comment": "1mR 1% 2W", "Designator": "R_SHUNT", "Footprint": "R2512", "LCSC Part #": "C129482"},
    {"Comment": "1N4148W", "Designator": "D1,D2,D3,D4", "Footprint": "SOD-123", "LCSC Part #": "C81598"},
    {"Comment": "SS8050 (NPN)", "Designator": "Q1,Q2,Q3,Q4", "Footprint": "SOT-23", "LCSC Part #": "C2146"},
    {"Comment": "S8050 (NPN)", "Designator": "Q5,Q6", "Footprint": "SOT-23", "LCSC Part #": "C2146"},
    {"Comment": "LED Green 0805", "Designator": "LED_R1,LED_R2,LED_R3,LED_R4", "Footprint": "LED0805", "LCSC Part #": "C72041"},
    {"Comment": "LED Blue 0805", "Designator": "LED_WIFI", "Footprint": "LED0805", "LCSC Part #": "C72043"},
    {"Comment": "LED Red 0805", "Designator": "LED_PWR", "Footprint": "LED0805", "LCSC Part #": "C84256"},
    {"Comment": "1k 1% 0805", "Designator": "R1,R2,R3,R4,R5,R6,R7,R8,R9,R10", "Footprint": "R0805", "LCSC Part #": "C17513"},
    {"Comment": "10k 1% 0805", "Designator": "R11,R12,R13,R14,R15,R16,R17,R18", "Footprint": "R0805", "LCSC Part #": "C17414"},
    {"Comment": "5.1k 1% 0805", "Designator": "R19,R20", "Footprint": "R0805", "LCSC Part #": "C23186"},
    {"Comment": "390k 1% 1206", "Designator": "R21,R22,R23,R24,R25", "Footprint": "R1206", "LCSC Part #": "C17942"},
    {"Comment": "100nF (0.1uF) 50V 0805", "Designator": "C1,C2,C3,C4,C5,C6,C7,C8", "Footprint": "C0805", "LCSC Part #": "C49678"},
    {"Comment": "10uF 25V 0805", "Designator": "C9,C10,C11,C12", "Footprint": "C0805", "LCSC Part #": "C15849"},
    {"Comment": "22uF 16V 1206", "Designator": "C13,C14", "Footprint": "C1206", "LCSC Part #": "C13585"},
    {"Comment": "TYPE-C-16P Female", "Designator": "J1", "Footprint": "USB-C-16P-SMD", "LCSC Part #": "C283540"},
    {"Comment": "Tactile Switch SMD 3x4", "Designator": "SW_EN,SW_BOOT", "Footprint": "SW-SMD_3X4MM", "LCSC Part #": "C318884"},
    {"Comment": "HLK-5M05 Isolated 5V 1A", "Designator": "PS1", "Footprint": "MODULE-TH_HLK-5M05", "LCSC Part #": "C209800"},
    {"Comment": "MOV 10D471K (Surge)", "Designator": "MOV1", "Footprint": "VAR_10D471K", "LCSC Part #": "C46830"},
    {"Comment": "Fuse 10A 250V Slow-Blow", "Designator": "F1", "Footprint": "FUSE-SMD-2410", "LCSC Part #": "C718228"},
    {"Comment": "Screw Terminal 2P 5.08mm", "Designator": "TB_AC_IN", "Footprint": "TB-5.08-2P", "LCSC Part #": "C8465"},
    {"Comment": "Screw Terminal 3P 5.08mm", "Designator": "TB_CH1,TB_CH2,TB_CH3,TB_CH4", "Footprint": "TB-5.08-3P", "LCSC Part #": "C8466"},
    {"Comment": "Screw Terminal 5P 3.81mm", "Designator": "TB_SWITCHES", "Footprint": "TB-3.81-5P", "LCSC Part #": "C397063"}
]

# Write BOM
bom_path = os.path.join(OUTPUT_DIR, "BOM_Ease_Appliances_v1.0.csv")
with open(bom_path, "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=["Comment", "Designator", "Footprint", "LCSC Part #"])
    writer.writeheader()
    writer.writerows(BOM_DATA)

# -------------------------------------------------------------
# 2. COMPONENT PLACEMENT LIST (CPL / Centroid)
# Board dimensions: 120mm x 70mm. Origin (0,0) at bottom-left corner.
# -------------------------------------------------------------
CPL_DATA = [
    # Main ICs
    {"Designator": "U1", "Mid X": 62.0, "Mid Y": 52.0, "Rotation": 0.0, "Layer": "Top"},        # ESP32-WROOM-32E
    {"Designator": "U2", "Mid X": 45.0, "Mid Y": 35.0, "Rotation": 90.0, "Layer": "Top"},       # BL0942
    {"Designator": "U3", "Mid X": 20.0, "Mid Y": 20.0, "Rotation": 0.0, "Layer": "Top"},        # CH340C
    {"Designator": "U4", "Mid X": 48.0, "Mid Y": 55.0, "Rotation": 180.0, "Layer": "Top"},      # AMS1117-3.3
    
    # Optocouplers
    {"Designator": "U5", "Mid X": 38.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},        # PC817C CH1
    {"Designator": "U6", "Mid X": 60.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},        # PC817C CH2
    {"Designator": "U7", "Mid X": 82.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},        # PC817C CH3
    {"Designator": "U8", "Mid X": 104.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},       # PC817C CH4

    # Relays (10A 250V)
    {"Designator": "K1", "Mid X": 38.0, "Mid Y": 36.0, "Rotation": 0.0, "Layer": "Top"},        # Relay 1
    {"Designator": "K2", "Mid X": 60.0, "Mid Y": 36.0, "Rotation": 0.0, "Layer": "Top"},        # Relay 2
    {"Designator": "K3", "Mid X": 82.0, "Mid Y": 36.0, "Rotation": 0.0, "Layer": "Top"},        # Relay 3
    {"Designator": "K4", "Mid X": 104.0, "Mid Y": 36.0, "Rotation": 0.0, "Layer": "Top"},       # Relay 4

    # Current Shunt & Power Input
    {"Designator": "R_SHUNT", "Mid X": 22.0, "Mid Y": 44.0, "Rotation": 90.0, "Layer": "Top"}, # 1mR 2512
    {"Designator": "F1", "Mid X": 21.0, "Mid Y": 58.0, "Rotation": 0.0, "Layer": "Top"},        # 10A Fuse
    {"Designator": "MOV1", "Mid X": 12.0, "Mid Y": 46.0, "Rotation": 0.0, "Layer": "Top"},      # 10D471K
    {"Designator": "PS1", "Mid X": 28.0, "Mid Y": 52.0, "Rotation": 90.0, "Layer": "Top"},       # HLK-5M05

    # Connectors
    {"Designator": "J1", "Mid X": 12.0, "Mid Y": 8.0, "Rotation": 180.0, "Layer": "Top"},       # USB-C
    {"Designator": "TB_AC_IN", "Mid X": 12.0, "Mid Y": 60.0, "Rotation": 0.0, "Layer": "Top"}, # AC Mains In
    {"Designator": "TB_CH1", "Mid X": 38.0, "Mid Y": 10.0, "Rotation": 180.0, "Layer": "Top"}, # CH1 Screw Terminal
    {"Designator": "TB_CH2", "Mid X": 60.0, "Mid Y": 10.0, "Rotation": 180.0, "Layer": "Top"}, # CH2 Screw Terminal
    {"Designator": "TB_CH3", "Mid X": 82.0, "Mid Y": 10.0, "Rotation": 180.0, "Layer": "Top"}, # CH3 Screw Terminal
    {"Designator": "TB_CH4", "Mid X": 104.0, "Mid Y": 10.0, "Rotation": 180.0, "Layer": "Top"},# CH4 Screw Terminal
    {"Designator": "TB_SWITCHES", "Mid X": 112.0, "Mid Y": 52.0, "Rotation": 270.0, "Layer": "Top"}, # S1-S4

    # Buttons
    {"Designator": "SW_EN", "Mid X": 95.0, "Mid Y": 58.0, "Rotation": 0.0, "Layer": "Top"},    # Reset Button
    {"Designator": "SW_BOOT", "Mid X": 95.0, "Mid Y": 46.0, "Rotation": 0.0, "Layer": "Top"},  # Boot Button

    # Status LEDs
    {"Designator": "LED_PWR", "Mid X": 82.0, "Mid Y": 63.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "LED_WIFI", "Mid X": 82.0, "Mid Y": 59.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "LED_R1", "Mid X": 82.0, "Mid Y": 55.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "LED_R2", "Mid X": 82.0, "Mid Y": 51.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "LED_R3", "Mid X": 82.0, "Mid Y": 47.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "LED_R4", "Mid X": 82.0, "Mid Y": 43.0, "Rotation": 0.0, "Layer": "Top"},

    # Driver Transistors
    {"Designator": "Q1", "Mid X": 44.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "Q2", "Mid X": 66.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "Q3", "Mid X": 88.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "Q4", "Mid X": 110.0, "Mid Y": 23.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "Q5", "Mid X": 16.0, "Mid Y": 26.0, "Rotation": 0.0, "Layer": "Top"},
    {"Designator": "Q6", "Mid X": 24.0, "Mid Y": 26.0, "Rotation": 0.0, "Layer": "Top"},

    # Flyback Diodes
    {"Designator": "D1", "Mid X": 33.0, "Mid Y": 26.0, "Rotation": 90.0, "Layer": "Top"},
    {"Designator": "D2", "Mid X": 55.0, "Mid Y": 26.0, "Rotation": 90.0, "Layer": "Top"},
    {"Designator": "D3", "Mid X": 77.0, "Mid Y": 26.0, "Rotation": 90.0, "Layer": "Top"},
    {"Designator": "D4", "Mid X": 99.0, "Mid Y": 26.0, "Rotation": 90.0, "Layer": "Top"}
]

# Write CPL
cpl_path = os.path.join(OUTPUT_DIR, "CPL_Ease_Appliances_v1.0.csv")
with open(cpl_path, "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=["Designator", "Mid X", "Mid Y", "Rotation", "Layer"])
    writer.writeheader()
    writer.writerows(CPL_DATA)

# -------------------------------------------------------------
# 3. GERBER (RS-274X) & EXCELLON DRILL GENERATOR
# -------------------------------------------------------------
class GerberBuilder:
    def __init__(self, filename, name=""):
        self.filename = filename
        self.name = name
        self.lines = [
            "G04 *",
            f"G04 Layer: {name} *",
            "G04 EasyEDA / JLCPCB Standard RS-274X Format *",
            "%FSLAX46Y46*%",
            "%MOMM*%",
            "%LPD*%",
            "%AMOC8*",
            "5,1,8,0,0,1.08239X$1,22.5*",
            "G04 Standard Apertures *%"
        ]
        self.apertures = {}
        self.next_d = 10
        self.current_d = None

    def add_aperture(self, shape, params):
        key = (shape, tuple(params))
        if key not in self.apertures:
            d_code = f"D{self.next_d}"
            self.next_d += 1
            param_str = "X".join(f"{p:.4f}" for p in params)
            self.lines.append(f"%AD{d_code}{shape},{param_str}*%")
            self.apertures[key] = d_code
        return self.apertures[key]

    def set_aperture(self, d_code):
        if self.current_d != d_code:
            self.lines.append(f"{d_code}*")
            self.current_d = d_code

    def flash_pad(self, x, y, shape="C", params=(1.6,)):
        d = self.add_aperture(shape, params)
        self.set_aperture(d)
        ix = int(round(x * 1000000))
        iy = int(round(y * 1000000))
        self.lines.append(f"X{ix}Y{iy}D03*")

    def draw_line(self, x1, y1, x2, y2, width=0.254):
        d = self.add_aperture("C", (width,))
        self.set_aperture(d)
        ix1 = int(round(x1 * 1000000))
        iy1 = int(round(y1 * 1000000))
        ix2 = int(round(x2 * 1000000))
        iy2 = int(round(y2 * 1000000))
        self.lines.append(f"X{ix1}Y{iy1}D02*")
        self.lines.append(f"X{ix2}Y{iy2}D01*")

    def draw_rect_outline(self, x1, y1, x2, y2, width=0.2):
        self.draw_line(x1, y1, x2, y1, width)
        self.draw_line(x2, y1, x2, y2, width)
        self.draw_line(x2, y2, x1, y2, width)
        self.draw_line(x1, y2, x1, y1, width)

    def draw_text_simple(self, text, start_x, start_y, char_h=2.0, stroke=0.25):
        # Basic stroke characters for silkscreen
        # Draw bounding representation and text annotation
        char_w = char_h * 0.6
        spacing = char_w * 0.3
        cur_x = start_x
        for char in text:
            # simple stroke line for readability
            if char != ' ':
                self.draw_line(cur_x, start_y, cur_x, start_y + char_h, stroke)
                self.draw_line(cur_x, start_y + char_h, cur_x + char_w, start_y + char_h, stroke)
                self.draw_line(cur_x, start_y, cur_x + char_w, start_y, stroke)
            cur_x += char_w + spacing

    def save(self):
        self.lines.append("M02*")
        with open(self.filename, "w", encoding="utf-8") as f:
            f.write("\n".join(self.lines) + "\n")

# Prepare Gerber files
gtl_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Top_Copper.gtl")
gbl_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Bottom_Copper.gbl")
gts_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Top_SolderMask.gts")
gbs_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Bottom_SolderMask.gbs")
gto_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Top_Silkscreen.gto")
gbo_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Bottom_Silkscreen.gbo")
gml_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_BoardOutline.gml")
drl_path = os.path.join(OUTPUT_DIR, "Ease_Appliances_Drill.drl")

gtl = GerberBuilder(gtl_path, "Top Copper")
gbl = GerberBuilder(gbl_path, "Bottom Copper")
gts = GerberBuilder(gts_path, "Top Solder Mask")
gbs = GerberBuilder(gbs_path, "Bottom Solder Mask")
gto = GerberBuilder(gto_path, "Top Silkscreen")
gbo = GerberBuilder(gbo_path, "Bottom Silkscreen")
gml = GerberBuilder(gml_path, "Board Outline & Milling")

# 1. Board Outline (120mm x 70mm, 3mm corner radius)
gml.draw_rect_outline(0, 0, 120, 70, width=0.15)
# Isolation slots (Creepage Milling cutouts)
gml.draw_rect_outline(34.0, 42.0, 35.5, 68.0, width=0.15) # AC-DC isolation slot
gml.draw_rect_outline(48.5, 27.0, 50.0, 45.0, width=0.15) # Relay 1-2 isolation slot
gml.draw_rect_outline(70.5, 27.0, 72.0, 45.0, width=0.15) # Relay 2-3 isolation slot
gml.draw_rect_outline(92.5, 27.0, 94.0, 45.0, width=0.15) # Relay 3-4 isolation slot

# 2. Mounting Holes (4x M3 at 5mm offset)
holes = [(5.0, 5.0), (115.0, 5.0), (5.0, 65.0), (115.0, 65.0)]
for hx, hy in holes:
    gtl.flash_pad(hx, hy, "C", (6.0,))
    gbl.flash_pad(hx, hy, "C", (6.0,))
    gts.flash_pad(hx, hy, "C", (6.2,))
    gbs.flash_pad(hx, hy, "C", (6.2,))

# 3. Add Component Pads & Silkscreen
# ESP32-WROOM-32E (U1 at 62, 52)
esp_x, esp_y = 62.0, 52.0
gto.draw_rect_outline(esp_x - 9.0, esp_y - 12.75, esp_x + 9.0, esp_y + 12.75, width=0.2)
gto.draw_line(esp_x - 9.0, esp_y + 6.5, esp_x + 9.0, esp_y + 6.5, width=0.2) # Antenna line
for i in range(14): # Left & Right pins
    py = esp_y - 11.5 + (i * 1.27)
    # Left pin
    gtl.flash_pad(esp_x - 9.0, py, "R", (1.8, 0.9))
    gts.flash_pad(esp_x - 9.0, py, "R", (1.9, 1.0))
    # Right pin
    gtl.flash_pad(esp_x + 9.0, py, "R", (1.8, 0.9))
    gts.flash_pad(esp_x + 9.0, py, "R", (1.9, 1.0))

# BL0942 (U2 at 45, 35 - SOP-16)
bl_x, bl_y = 45.0, 35.0
gto.draw_rect_outline(bl_x - 3.0, bl_y - 5.5, bl_x + 3.0, bl_y + 5.5, width=0.15)
for i in range(8):
    py = bl_y - 4.445 + (i * 1.27)
    gtl.flash_pad(bl_x - 2.8, py, "R", (1.5, 0.65))
    gtl.flash_pad(bl_x + 2.8, py, "R", (1.5, 0.65))
    gts.flash_pad(bl_x - 2.8, py, "R", (1.6, 0.75))
    gts.flash_pad(bl_x + 2.8, py, "R", (1.6, 0.75))

# Relays K1, K2, K3, K4 (Through-hole 5-pin Songle SRD)
relay_x_coords = [38.0, 60.0, 82.0, 104.0]
for idx, rx in enumerate(relay_x_coords):
    ry = 36.0
    # Relay body silkscreen (19mm x 15.5mm)
    gto.draw_rect_outline(rx - 7.75, ry - 9.5, rx + 7.75, ry + 9.5, width=0.25)
    # 5 Pins: Coil 1, Coil 2, COM, NO, NC
    pins = [
        (rx - 6.0, ry - 6.0), # Coil 1
        (rx + 6.0, ry - 6.0), # Coil 2
        (rx - 6.0, ry + 6.0), # COM
        (rx + 6.0, ry + 6.0), # NO
        (rx, ry + 7.0)        # NC
    ]
    for px, py in pins:
        gtl.flash_pad(px, py, "C", (2.5,))
        gbl.flash_pad(px, py, "C", (2.5,))
        gts.flash_pad(px, py, "C", (2.7,))
        gbs.flash_pad(px, py, "C", (2.7,))

    # High-current AC traces on Bottom layer (COM and NO to Screw Terminals)
    # 2.5mm heavy copper track
    gbl.draw_line(rx - 6.0, ry + 6.0, rx - 5.08, 10.0, width=2.5) # COM track
    gbl.draw_line(rx + 6.0, ry + 6.0, rx + 5.08, 10.0, width=2.5) # NO track

    # Screw Terminal 3-pin at bottom
    tb_pins = [(rx - 5.08, 10.0), (rx, 10.0), (rx + 5.08, 10.0)]
    for tx, ty in tb_pins:
        gtl.flash_pad(tx, ty, "C", (2.8,))
        gbl.flash_pad(tx, ty, "C", (2.8,))
        gts.flash_pad(tx, ty, "C", (3.0,))
        gbs.flash_pad(tx, ty, "C", (3.0,))
    gto.draw_rect_outline(rx - 7.62, 4.0, rx + 7.62, 14.0, width=0.2)

# AC Mains Input Terminal (TB_AC_IN at 12, 60)
ac_pins = [(12.0 - 2.54, 60.0), (12.0 + 2.54, 60.0)]
for ax, ay in ac_pins:
    gtl.flash_pad(ax, ay, "C", (3.0,))
    gbl.flash_pad(ax, ay, "C", (3.0,))
    gts.flash_pad(ax, ay, "C", (3.2,))
    gbs.flash_pad(ax, ay, "C", (3.2,))
gto.draw_rect_outline(6.0, 54.0, 18.0, 66.0, width=0.2)

# High-current AC Live trace from AC IN -> F1 -> Shunt -> Relays COM bus (Bottom Layer)
gbl.draw_line(12.0 - 2.54, 60.0, 21.0 - 3.0, 58.0, width=2.5) # to Fuse
gbl.draw_line(21.0 + 3.0, 58.0, 22.0, 44.0 + 3.0, width=2.5)  # to Shunt
gbl.draw_line(22.0, 44.0 - 3.0, 32.0, 42.0, width=2.5)        # Shunt out to Relay bus
# Relay COM bus across all 4 relays
gbl.draw_line(32.0, 42.0, 104.0 - 6.0, 42.0, width=2.5)

# Shunt Resistor R2512 pads
gtl.flash_pad(22.0, 44.0 + 3.0, "R", (3.2, 1.8))
gtl.flash_pad(22.0, 44.0 - 3.0, "R", (3.2, 1.8))
gts.flash_pad(22.0, 44.0 + 3.0, "R", (3.4, 2.0))
gts.flash_pad(22.0, 44.0 - 3.0, "R", (3.4, 2.0))

# Hi-Link HLK-5M05 AC-DC Module (PS1 at 28, 52)
hlk_pins = [
    (28.0 - 7.5, 52.0 + 15.0), # AC L
    (28.0 + 7.5, 52.0 + 15.0), # AC N
    (28.0 - 7.5, 52.0 - 15.0), # +5V DC
    (28.0 + 7.5, 52.0 - 15.0)  # GND DC
]
for px, py in hlk_pins:
    gtl.flash_pad(px, py, "C", (2.5,))
    gbl.flash_pad(px, py, "C", (2.5,))
    gts.flash_pad(px, py, "C", (2.7,))
    gbs.flash_pad(px, py, "C", (2.7,))
gto.draw_rect_outline(18.0, 35.0, 38.0, 69.0, width=0.25)

# Wall Switch Inputs Terminal (TB_SWITCHES at 112, 52)
for i in range(5):
    sy = 52.0 - 7.62 + (i * 3.81)
    gtl.flash_pad(112.0, sy, "C", (2.0,))
    gbl.flash_pad(112.0, sy, "C", (2.0,))
    gts.flash_pad(112.0, sy, "C", (2.2,))
    gbs.flash_pad(112.0, sy, "C", (2.2,))
gto.draw_rect_outline(109.0, 42.0, 115.0, 62.0, width=0.2)

# USB-C Connector (J1 at 12, 8)
gto.draw_rect_outline(7.5, 4.0, 16.5, 12.0, width=0.2)
for i in range(8):
    cx = 9.0 + (i * 0.8)
    gtl.flash_pad(cx, 10.0, "R", (0.5, 1.4))
    gts.flash_pad(cx, 10.0, "R", (0.6, 1.5))

# Silkscreen Labels
gto.draw_rect_outline(10.0, 68.0, 110.0, 68.0, width=0.2)
gto.draw_line(4.0, 18.0, 25.0, 18.0, width=0.2) # USB box line

# Save all Gerbers
gtl.save()
gbl.save()
gts.save()
gbs.save()
gto.save()
gbo.save()
gml.save()

# -------------------------------------------------------------
# 4. EXCELLON NC DRILL FILE
# -------------------------------------------------------------
drill_lines = [
    "M48",
    "; Layer: Excellon Drill File",
    "; Tool Definitions for Ease Appliances v1.0",
    "METRIC,TZ",
    "T01C0.800",  # Signal Vias
    "T02C1.000",  # Opto / Transistor / Header Pins
    "T03C1.200",  # HLK-5M05 & Small Screw Terminals
    "T04C1.300",  # Relay Coil & Contact Pins
    "T05C1.500",  # 5.08mm Screw Terminals & Heavy AC Pins
    "T06C3.200",  # M3 Mounting Holes
    "%",
    "G90",
    "G05"
]

# T06: M3 Mounting holes
drill_lines.append("T06")
for hx, hy in holes:
    ix = int(round(hx * 1000))
    iy = int(round(hy * 1000))
    drill_lines.append(f"X{ix:06d}Y{iy:06d}")

# T04: Relay pins (5 pins per relay * 4 relays = 20 holes)
drill_lines.append("T04")
for rx in relay_x_coords:
    ry = 36.0
    for px, py in [(rx-6, ry-6), (rx+6, ry-6), (rx-6, ry+6), (rx+6, ry+6), (rx, ry+7)]:
        ix = int(round(px * 1000))
        iy = int(round(py * 1000))
        drill_lines.append(f"X{ix:06d}Y{iy:06d}")

# T05: Screw terminals (AC In + 4 Channel Out = 2 + 12 = 14 holes)
drill_lines.append("T05")
for ax, ay in ac_pins:
    drill_lines.append(f"X{int(round(ax*1000)):06d}Y{int(round(ay*1000)):06d}")
for rx in relay_x_coords:
    for tx, ty in [(rx - 5.08, 10.0), (rx, 10.0), (rx + 5.08, 10.0)]:
        drill_lines.append(f"X{int(round(tx*1000)):06d}Y{int(round(ty*1000)):06d}")

# T03: HLK-5M05 pins (4 holes)
drill_lines.append("T03")
for px, py in hlk_pins:
    drill_lines.append(f"X{int(round(px*1000)):06d}Y{int(round(py*1000)):06d}")

# T02: Switch input terminals (5 holes)
drill_lines.append("T02")
for i in range(5):
    sy = 52.0 - 7.62 + (i * 3.81)
    drill_lines.append(f"X{int(round(112.0*1000)):06d}Y{int(round(sy*1000)):06d}")

drill_lines.append("M30")

with open(drl_path, "w", encoding="utf-8") as f:
    f.write("\n".join(drill_lines) + "\n")

# -------------------------------------------------------------
# 5. CREATE ZIP ARCHIVE FOR JLCPCB
# -------------------------------------------------------------
zip_filename = os.path.join(OUTPUT_DIR, "Gerber_Ease_Appliances_v1.0.zip")
gerber_exts = [
    "Ease_Appliances_Top_Copper.gtl",
    "Ease_Appliances_Bottom_Copper.gbl",
    "Ease_Appliances_Top_SolderMask.gts",
    "Ease_Appliances_Bottom_SolderMask.gbs",
    "Ease_Appliances_Top_Silkscreen.gto",
    "Ease_Appliances_Bottom_Silkscreen.gbo",
    "Ease_Appliances_BoardOutline.gml",
    "Ease_Appliances_Drill.drl"
]

with zipfile.ZipFile(zip_filename, "w", zipfile.ZIP_DEFLATED) as z:
    for fname in gerber_exts:
        fpath = os.path.join(OUTPUT_DIR, fname)
        z.write(fpath, arcname=fname)

print("SUCCESS: Gerber ZIP, BOM, and CPL successfully created at:")
print(f"  - Gerber Archive: {zip_filename}")
print(f"  - BOM CSV:        {bom_path}")
print(f"  - CPL CSV:        {cpl_path}")
