# "My Smart Plug" — Figma Design System & Prototyping Guide

A complete, production-grade Figma design system and prototyping blueprint for the **"My Smart Plug"** mobile application. Built for a **390 × 844 px** viewport with a clean light aesthetic, subtle purple/indigo accents, and strictly consumer-friendly terminology.

---

## 1. Quick Import Into Figma

### Method A: Drag-and-Drop Vector Artboards (Recommended)
All artboards in `figma/artboards/` are formatted as 390 × 844 px SVGs with named vector groups (`Status_Bar`, `Header`, `Device_Card`, `Power_Dial`, `Actions`, etc.).
1. Open Figma and create a new design file.
2. Open your local file explorer at:
   ```
   /Users/rajnishmishra/.gemini/antigravity/scratch/my-smart-plug/figma/artboards/
   ```
3. Select all 14 SVG files and drag them directly onto the Figma canvas.
4. Figma will instantiate each screen as a standalone frame with editable text layers, vector shapes, gradients, and layer groups.

### Method B: Master Prototype Flow Canvas
Drag `figma/master_flow_canvas.svg` onto your Figma canvas. This imports all 14 screens arranged on a 5×3 grid complete with connection arrows and flow labels.

### Method C: Design Tokens Import (Tokens Studio)
If you use the **Tokens Studio for Figma** plugin:
1. Open Tokens Studio in your Figma file.
2. Select **Settings > Load from file** and pick `figma/figma_tokens.json`.
3. Click **Apply to Selection** or sync to create native Figma Color & Typography Styles.

---

## 2. Global Styles & Design Tokens

### Color Styles
| Token Name | Hex Code | Usage |
|---|---|---|
| `Primary / Main` | `#6366F1` | Primary buttons, active power dial glow, selected radio strokes |
| `Primary / Active` | `#4F46E5` | Active press states, gradient stop |
| `Primary / Light` | `#EEF2FF` | Highlighted card backgrounds, icon container fills |
| `Primary / Border` | `#C7D2FE` | Selected network border, badge stroke |
| `Surface / Canvas` | `#F8FAFC` | App screen background |
| `Surface / Card` | `#FFFFFF` | Device cards, setting lists, modal sheets |
| `Surface / Subtle` | `#F1F5F9` | Disabled power dials, card divider strokes |
| `Text / Primary` | `#0F172A` | Screen headers, plug titles, primary text |
| `Text / Secondary`| `#475569` | Body descriptions, checklist items |
| `Text / Muted` | `#94A3B8` | Subtitles, timestamps, step indicators |
| `Status / Connected`| `#10B981` | Emerald dot and badge fill (`#ECFDF5`) |
| `Status / Offline` | `#F59E0B` | Amber warning dot, offline banner fill (`#FFFBEB`) |
| `Status / Error` | `#EF4444` | Validation messages, danger button text, error banner |

### Typography Hierarchy (Font: Plus Jakarta Sans or SF Pro)
- **Display Large**: 28px, Bold (800), Line height 34px, Tracking -0.5px (Splash, Success titles)
- **Title Screen**: 26px, Bold (800), Line height 32px, Tracking -0.5px (Screen headers)
- **Section Heading**: 17px, SemiBold (700), Line height 24px (Device Card titles)
- **Body Default**: 14px, Medium (500), Line height 20px (Descriptions, network rows)
- **Caption**: 12px, Regular (400), Line height 16px (Subtitles, status badges)
- **Overline / Tiny**: 11px, Bold (700), Line height 14px, Letter spacing +1px (Step counters, badges)

---

## 3. Auto Layout Component Specs

### Component 1: `DeviceCard`
- **Frame Type**: Auto Layout (Vertical)
- **Width**: 342px | **Height**: Hug contents (min 152px)
- **Padding**: 20px all sides | **Gap**: 16px
- **Corner Radius**: 24px
- **Fill**: Solid `#FFFFFF` | **Stroke**: 1px `#F1F5F9`
- **Effects**: Drop shadow `0px 8px 24px rgba(15, 23, 42, 0.04)`
- **Variants**:
  - `State=ON`: Power switch thumb positioned right with `#6366F1` track fill.
  - `State=OFF`: Power switch thumb positioned left with `#E2E8F0` track fill.

### Component 2: `CircularPowerDial` (Device Control Screen)
- **Frame Type**: Frame (Center-aligned constraints)
- **Size**: 190 × 190 px (Outer ring 210px with halo)
- **Corner Radius**: 9999px (Circle)
- **Variants**:
  - `State=ON`:
    - Fill: Linear gradient from `#6366F1` (0%) to `#4338CA` (100%)
    - Stroke: 3px `#818CF8`
    - Effects: Drop shadow glow `0px 0px 48px rgba(99, 102, 241, 0.40)`
    - Label: "ON", text color `#FFFFFF`
    - Subtext: "Power is on", text color `#E0E7FF`
  - `State=OFF`:
    - Fill: Linear gradient from `#F1F5F9` to `#E2E8F0`
    - Stroke: 2px `#CBD5E1`
    - Effects: Drop shadow `0px 4px 12px rgba(15, 23, 42, 0.04)`
    - Label: "OFF", text color `#64748B`
    - Subtext: "Power is off", text color `#94A3B8`
  - `State=Disabled` (Offline):
    - Fill: Solid `#F1F5F9`
    - Stroke: 2px `#E2E8F0`, diagonal red slash across dial
    - Label: "OFFLINE", text color `#94A3B8`

### Component 3: `WifiNetworkRow`
- **Frame Type**: Auto Layout (Horizontal)
- **Width**: 342px | **Height**: 56px (Selected: 60px)
- **Padding**: Horizontal 18px, Vertical 14px
- **Corner Radius**: 18px
- **Variants**:
  - `Selected=True`: Fill `#EEF2FF`, Stroke 1.8px `#6366F1`, includes purple checkmark pill.
  - `Selected=False`: Fill `#FFFFFF`, Stroke 1px `#E2E8F0`, lock icon right-aligned.

### Component 4: `PrimaryButton`
- **Frame Type**: Auto Layout (Horizontal, Center-aligned)
- **Width**: 342px | **Height**: 56px
- **Corner Radius**: 20px
- **Fill**: Linear Gradient `#6366F1` to `#4F46E5`
- **Effects**: Drop shadow `0px 8px 20px rgba(99, 102, 241, 0.28)`
- **Typography**: 15px, Bold (700), Color `#FFFFFF`

---

## 4. Figma Prototype Interaction Wiring Map

Connect the screens in Figma's **Prototype Tab** using these exact triggers and animations:

| From Screen / Element | To Screen | Trigger | Action | Animation |
|---|---|---|---|---|
| **01. Splash** (Canvas) | `02_home_my_devices` | **After delay** (2500ms) | Navigate to | **Dissolve** (350ms, Ease-out) |
| **01. Splash** (Tap anywhere) | `02_home_my_devices` | **On tap** | Navigate to | **Instant** |
| **02. Home** (`+ Add Smart Plug`) | `03_add_smart_plug` | **On tap** | Navigate to | **Slide In** (from right, 300ms, Ease-out) |
| **02. Home** (Device Card body) | `08_device_control_on` | **On tap** | Navigate to | **Smart Animate** (300ms, Ease-out) |
| **02. Home** (Power Switch) | Toggle between ON/OFF | **On tap** | Change to | **Smart Animate** (200ms, Ease-out) |
| **03. Add** (`Btn_Back`) | `02_home_my_devices` | **On tap** | Navigate to | **Slide Out** (to right, 300ms) |
| **03. Add** (`Start Setup`) | `04_finding_smart_plug` | **On tap** | Navigate to | **Slide In** (from right, 300ms) |
| **04. Finding** (Canvas) | Shows detected plug | **After delay** (2500ms) | Change state | **Dissolve** (250ms) |
| **04. Finding** (`Continue`) | `05_wifi_setup` | **On tap** | Navigate to | **Slide In** (from right, 300ms) |
| **05. Wi-Fi Setup** (`Connect Smart Plug`) | `06_setting_up_progress` | **On tap** | Navigate to | **Smart Animate** (300ms) |
| **06. Setting Up** (Progress complete) | `07_smart_plug_ready` | **After delay** (3000ms) | Navigate to | **Smart Animate** (350ms, Ease-out) |
| **06. Setting Up** (If error simulation) | `06b_connection_failed` | **On tap** | Navigate to | **Dissolve** (250ms) |
| **06b. Connection Failed** (`Try Again`) | `06_setting_up_progress` | **On tap** | Navigate to | **Smart Animate** (250ms) |
| **06b. Connection Failed** (`Change Wi-Fi`)| `05_wifi_setup` | **On tap** | Navigate to | **Slide Out** (to right, 300ms) |
| **07. Ready** (`Start Using`) | `08_device_control_on` | **On tap** | Navigate to | **Slide In** (from bottom, 300ms) |
| **08. Device Control** (`Power_Dial`) | `08b_device_control_off` | **On tap** | Navigate to | **Smart Animate** (200ms, Quick) |
| **08b. Device Control OFF** (`Power_Dial`)| `08_device_control_on` | **On tap** | Navigate to | **Smart Animate** (200ms, Quick) |
| **08. Device Control** (`Btn_Settings`) | `09_device_settings` | **On tap** | Navigate to | **Slide In** (from right, 300ms) |
| **08. Device Control** (`Wi-Fi Change`) | `10_wifi_settings` | **On tap** | Navigate to | **Slide In** (from right, 300ms) |
| **09. Device Settings** (`Remove Plug`) | Modal Sheet | **On tap** | Open Overlay | **Move In** (from bottom, 250ms) |
| **Remove Modal** (`Confirm Remove`) | `12_empty_home_state` | **On tap** | Navigate to | **Crossfade** (300ms) |
| **10. Wi-Fi Settings** (`Change Wi-Fi`) | `05_wifi_setup` | **On tap** | Navigate to | **Slide In** (from right, 300ms) |
| **11. Offline State** (`Try Again`) | `08_device_control_on` | **On tap** | Navigate to | **Dissolve** (300ms) |
| **12. Empty Home** (`+ Add Smart Plug`) | `03_add_smart_plug` | **On tap** | Navigate to | **Slide In** (from right, 300ms) |

---

## 5. Visual Preview of Generated Screens

The visual mockups generated in `figma/assets/` demonstrate the exact design fidelity:

- `smart_plug_home_screen_*.jpg`: Home screen with clean device card, status badge, and glowing purple power switch.
- `smart_plug_device_control_*.jpg`: Large circular power dial with glowing radial depth and status info.
- `smart_plug_wifi_setup_*.jpg`: Wi-Fi selection list with password input and validation layout.
