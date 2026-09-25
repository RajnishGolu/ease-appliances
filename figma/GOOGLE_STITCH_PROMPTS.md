# Google Stitch Prompts — "My Smart Plug" Mobile Prototype

These prompts are tailored specifically for **Google Stitch** (Google Labs' AI UI design & prototyping tool) to generate high-fidelity, production-ready mobile screens that export directly to **Figma**.

---

## 1. Master Project Prompt (All-In-One Flow)

Paste this into Google Stitch's main prompt bar to generate the entire multi-screen flow:

```text
Design a high-fidelity, modern mobile smart-home app prototype called "My Smart Plug" for iOS (frame size: 390x844 px). 

Design Target & Aesthetic:
- Clean, minimalist Apple Home and Google Home aesthetic.
- Light theme: Crisp white (#FFFFFF) and soft slate (#F8FAFC) background with subtle violet-indigo accent (#6366F1).
- Rounded modern cards (radius 20-24px), tactile buttons (height 54px, radius 20px), and clear typography using Plus Jakarta Sans or SF Pro.
- Strictly consumer-friendly terminology: use "Smart Plug", "Power", "Wi-Fi", "Connected", "Offline", "Setup". Never show technical terms like ESP8266, MQTT, GPIO, Relay, AWS IoT, or broker.

Complete 5-Screen Core Flow:

Screen 1 - Home / My Devices:
Top header with "Good evening", profile avatar, and title "My Smart Plugs". A rounded device card for "Living Room" showing an emerald green "Connected" status pill with signal icon, an interactive tactile ON/OFF toggle switch in glowing violet (#6366F1), current state "ON", and timestamp "Updated just now". Below the card, show an ambient smart tip card, and a prominent bottom button "+ Add Smart Plug".

Screen 2 - Add Smart Plug (Step 1):
Back button, header "Add your Smart Plug", description "Let's connect your Smart Plug to your home Wi-Fi." Clean vector illustration of a minimalist smart plug with glowing power LED and electrical pins. Guidance checklist (1. Plug into wall, 2. Check blinking light). Primary button "Start Setup", secondary text "Already configured? Connect existing plug".

Screen 3 - Finding & Wi-Fi Setup:
Top header "Connect to Wi-Fi", subtitle "Choose your home Wi-Fi network". List of Wi-Fi networks with signal bars and lock icons: "Home Wi-Fi" (highlighted with purple border and checkmark), "Rajnish Home", "Office Wi-Fi". A "Refresh networks" button. Below the list, a password field "Wi-Fi Password" with eye show/hide icon, a "Remember this network" checkbox, and a primary button "Connect Smart Plug".

Screen 4 - Setting Up Progress (Step 3):
Title "Setting up your Smart Plug", subtitle "This may take a few seconds." A 4-stage sequential progress stepper: 
1. ✓ Smart Plug found (green check)
2. ✓ Wi-Fi configured (green check)
3. ● Connecting to your home network (active purple spinning pulse)
4. ○ Getting ready (pending gray)
Animated progress bar at 75%. Also provide an error alternative card "Couldn't connect. Check your Wi-Fi password and try again" with "Try Again" and "Change Wi-Fi" buttons.

Screen 5 - Device Control (ON State):
Top navigation with back chevron, title "Living Room", and gear settings icon. Emerald pill "Connected". In the center, a large 190px circular tactile power button glowing with an electric violet-purple radial gradient, white power symbol, bold label "ON", and subtext "Power is on". Below, cards showing "Wi-Fi: Strong (Home Wi-Fi)" and quick action pills for "Rename" and "Wi-Fi Settings".
```

---

## 2. Google Stitch `DESIGN.md` (Design Tokens Specification)

If using Google Stitch's design system or `DESIGN.md` feature, paste this specification:

```markdown
# DESIGN.md - My Smart Plug

## Design Philosophy
Consumer-first, minimalist smart-home interface. The interface prioritizes calm, effortless home control without any technical or IoT jargon.

## Visual Tokens
- **Canvas / Background**: #F8FAFC
- **Surface / Card**: #FFFFFF
- **Surface / Subtle**: #F1F5F9
- **Brand Primary**: #6366F1 (Indigo 500)
- **Brand Active / Hover**: #4F46E5 (Indigo 600)
- **Brand Soft Fill**: #EEF2FF (Indigo 50)
- **Power Dial Glow**: rgba(99, 102, 241, 0.40)
- **Text Primary**: #0F172A (Slate 900)
- **Text Secondary**: #475569 (Slate 600)
- **Text Muted**: #94A3B8 (Slate 400)
- **Status Connected**: #10B981 (Emerald 500) on #ECFDF5
- **Status Offline**: #F59E0B (Amber 500) on #FFFBEB
- **Status Error / Danger**: #EF4444 (Rose 500) on #FEF2F2

## Component Rules
- **Device Frame**: iPhone mobile viewport (390 x 844 px).
- **Corner Radii**: Cards 24px, Buttons 20px, Badges 9999px (full pill).
- **Card Shadows**: 0 8px 24px -4px rgba(15, 23, 42, 0.04), border: 1px solid #F1F5F9.
- **Typography**: Display 28px/800, Screen Title 24px/800, Section Heading 17px/700, Body 14px/500, Caption 12px/400.
- **Terminology Constraints**: STRICTLY consumer terms (Smart Plug, Power, Wi-Fi, Connected, Offline, Setup). Zero technical jargon.
```

---

## 3. Modular Screen-by-Screen Prompts

Use these prompts in Stitch to generate or refine individual screens:

### Screen 1: Home / My Devices
```text
Mobile iOS screen (390x844 px), light background (#F8FAFC). Header with 'Good evening', user avatar 'RM', and title 'My Smart Plugs'. A premium rounded card for 'Living Room' with plug icon, emerald 'Connected' status pill, Wi-Fi icon, toggle switch in ON state (purple #6366F1), state text 'ON', and 'Updated just now'. Below, a soft purple card with 'Tap card to inspect', and a primary button '+ Add Smart Plug'. Apple Home aesthetic.
```

### Screen 2: Add Smart Plug
```text
Mobile iOS screen (390x844 px), clean white background. Top bar with back button, step indicator 'Step 1 of 3'. Title 'Add your Smart Plug', subtitle 'Let's connect your Smart Plug to your home Wi-Fi.' In the center, a clean modern illustration of a white smart plug with glowing purple power LED and electrical pins. Below are two numbered setup steps: '1. Plug into electrical wall socket', '2. Verify indicator light is blinking'. Primary button 'Start Setup', secondary text 'Already configured? Connect existing plug'.
```

### Screen 3: Finding Smart Plug (Radar Discovery)
```text
Mobile iOS screen (390x844 px), light background. Step indicator 'Step 2 of 3'. Title 'Looking for your Smart Plug', subtitle 'Make sure your Smart Plug is powered on.' Center area displays a pulsing circular radar animation with concentric indigo waves radiating from a glowing purple smart plug icon. Status badge below: 'Searching nearby...' with pulsing indicator. Card below: 'Smart Plug found • SP-001' with green checkmark. Bottom primary button 'Continue'.
```

### Screen 4: Wi-Fi Setup & Password
```text
Mobile iOS screen (390x844 px), light background. Step indicator 'Step 3 of 3'. Title 'Connect to Wi-Fi', subtitle 'Choose your home Wi-Fi network.' Section 'Available Networks' with a 'Refresh networks' button. List of Wi-Fi rows with lock icons: 'Home Wi-Fi' (selected with purple border and checkmark), 'Rajnish Home', 'Office Wi-Fi', 'Other Network'. Below, a card with Wi-Fi Password input field, show/hide eye toggle, and 'Remember this network' checkbox. Bottom primary button 'Connect Smart Plug'.
```

### Screen 5: Setting Up Progress & Error State
```text
Mobile iOS screen (390x844 px), white background. Title 'Setting up your Smart Plug', subtitle 'This may take a few seconds.' Top 75% animated progress bar. 4-step vertical stepper: 
1. ✓ Smart Plug found (green check)
2. ✓ Wi-Fi configured (green check)
3. ● Connecting to your home network (active purple spinning pulse)
4. ○ Getting ready (gray pending)
Also show a secondary alternative error modal: 'Couldn't connect. Check your Wi-Fi password and try again' with 'Try Again' and 'Change Wi-Fi' buttons.
```

### Screen 6: Smart Plug Ready (Success)
```text
Mobile iOS screen (390x844 px), clean white background. Large emerald green circular checkmark in the center with subtle purple sparkle particles. Title 'You're all set!', subtitle 'Your Smart Plug is connected and ready to use.' A rounded summary card showing 'Living Room' with green 'Connected' badge. Bottom primary button 'Start Using', secondary text 'Rename Smart Plug'.
```

### Screen 7: Device Control (Circular Power Dial)
```text
Mobile iOS screen (390x844 px), light slate background (#F8FAFC). Top bar with back chevron, title 'Living Room', and gear settings icon. Emerald status badge 'Connected'. In the center, a massive 190px tactile circular power dial glowing with a radial violet-purple gradient (#6366F1), power icon, label 'ON', and subtext 'Power is on'. Below, a card with 'Wi-Fi Connection: Strong (Home Wi-Fi)' and action tiles for 'Rename' and 'Wi-Fi Settings'. Bottom toast notification 'Power turned on'.
```

### Screen 8: Offline State
```text
Mobile iOS screen (390x844 px), light background. Top bar with back button and title 'Living Room'. Amber warning banner at top: 'Smart Plug Offline • We can't reach your Smart Plug right now.' The large circular power dial is grayed out, disabled, with a subtle diagonal red slash and text 'OFFLINE - Controls disabled'. Bottom action buttons: primary button 'Try Again' and secondary button 'Reconnect Wi-Fi'.
```

### Screen 9: Device Settings & Remove Modal
```text
Mobile iOS screen (390x844 px), light background. Title 'Smart Plug Settings'. White rounded card with settings list: 'Smart Plug Name: Living Room', 'Wi-Fi: Home Wi-Fi', 'Connection Status: Connected', 'Firmware: v2.4.1'. At bottom, a soft red danger button 'Remove Smart Plug'. Display a bottom-sheet confirmation modal over the screen: 'Remove this Smart Plug? This will disconnect it from your account' with 'Cancel' and 'Remove' buttons.
```

### Screen 10: Empty Home State
```text
Mobile iOS screen (390x844 px), light background. Header 'Good evening', title 'My Smart Plugs'. In the center, a minimalist vector illustration of an unplugged smart plug floating in a soft purple circle. Title 'No Smart Plugs yet', subtitle 'Add your first Smart Plug to get started.' Bottom primary button '+ Add Smart Plug'.
```
