import subprocess
import json
import time

def get_real_wifi_networks():
    networks = []
    seen = set()

    # 1. Parse both Current and Other Local Over-The-Air Wi-Fi Networks from macOS
    try:
        out = subprocess.check_output(['system_profiler', 'SPAirPortDataType'], text=True, timeout=10)
        lines = out.splitlines()
        current_section = None

        for line in lines:
            stripped = line.strip()
            if not stripped:
                continue

            indent = len(line) - len(line.lstrip())

            if 'Current Network Information:' in line:
                current_section = 'current'
                continue
            elif 'Other Local Wi-Fi Networks:' in line:
                current_section = 'other'
                continue
            elif indent <= 8:
                current_section = None

            if current_section in ('current', 'other'):
                # In system_profiler, SSID names are at indent 12 (under en0) and end with ':'
                if indent == 12 and stripped.endswith(':'):
                    cand = stripped[:-1].strip()
                    # Filter out property keys
                    if cand and not any(k in cand for k in [
                        'PHY Mode', 'Channel', 'Security', 'Network Type', 'Signal', 'Country Code', 'MCS Index'
                    ]):
                        if cand not in seen:
                            seen.add(cand)
                            is_curr = (current_section == 'current')
                            networks.append({
                                'ssid': cand,
                                'signal': 'Strong' if is_curr else 'In Range',
                                'isCurrent': is_curr
                            })
    except Exception as e:
        print("system_profiler scan error:", e)

    # 2. Get saved / preferred wireless networks as fallback
    try:
        out = subprocess.check_output(['networksetup', '-listpreferredwirelessnetworks', 'en0'], text=True, timeout=5)
        for line in out.splitlines():
            line_str = line.strip()
            if not line_str or line_str.startswith('Preferred networks'):
                continue
            if line_str not in seen:
                seen.add(line_str)
                networks.append({
                    'ssid': line_str,
                    'signal': 'Saved',
                    'isCurrent': False
                })
    except Exception as e:
        print("Preferred networks error:", e)

    # Return all unique detected networks
    return networks[:35]

if __name__ == '__main__':
    nets = get_real_wifi_networks()
    print(f"Total networks found: {len(nets)}")
    for n in nets:
        tag = 'CURRENT' if n['isCurrent'] else n['signal']
        print(f" - {n['ssid']} [{tag}]")
