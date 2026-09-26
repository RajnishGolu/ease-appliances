import http.server
import socketserver
import json
import os
import urllib.request
import urllib.error
import urllib.parse
import scan_wifi

PORT = 3000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))
IP_CACHE_FILE = os.path.join(DIRECTORY, "esp32_ip.txt")

def get_known_ip():
    if os.path.exists(IP_CACHE_FILE):
        try:
            with open(IP_CACHE_FILE, "r") as f:
                ip = f.read().strip()
                if ip:
                    return ip
        except Exception:
            pass
    return "192.168.0.183"

def save_known_ip(ip):
    try:
        with open(IP_CACHE_FILE, "w") as f:
            f.write(ip.strip())
    except Exception:
        pass

class PWAHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        path = parsed.path
        query = urllib.parse.parse_qs(parsed.query)

        # 1. API endpoint for real Wi-Fi networks from host system
        if path == '/api/wifi-networks':
            networks = scan_wifi.get_real_wifi_networks()
            self._send_json(networks)
            return

        # 2. API endpoint to query ESP32 status over LAN
        if path == '/api/esp32/status':
            target_ip = query.get('ip', [get_known_ip()])[0]
            try:
                req = urllib.request.Request(f"http://{target_ip}/status", headers={'User-Agent': 'SmartPlugApp/1.0', 'Connection': 'close'})
                with urllib.request.urlopen(req, timeout=3.5) as resp:
                    data = json.loads(resp.read().decode('utf-8'))
                    save_known_ip(target_ip)
                    self._send_json(data)
                    return
            except Exception as e:
                self._send_json({"online": False, "ip": target_ip, "error": str(e)}, status=200)
                return

        # 3. API endpoint to toggle or set ESP32 relay over LAN
        if path == '/api/esp32/relay':
            target_ip = query.get('ip', [get_known_ip()])[0]
            state = query.get('state', ['toggle'])[0]
            ch = query.get('ch', query.get('channel', ['0']))[0]
            try:
                url = f"http://{target_ip}/relay?ch={ch}&state={state}" if state in ['on', 'off', '0', '1', 'TRUE', 'FALSE'] else f"http://{target_ip}/relay?ch={ch}&toggle=1"
                req = urllib.request.Request(url, headers={'User-Agent': 'SmartPlugApp/1.0', 'Connection': 'close'})
                with urllib.request.urlopen(req, timeout=3.5) as resp:
                    data = json.loads(resp.read().decode('utf-8'))
                    self._send_json(data)
                    return
            except Exception as e:
                self._send_json({"success": False, "error": str(e)}, status=500)
                return

        # 4. Save known IP
        if path == '/api/esp32/set-ip':
            new_ip = query.get('ip', [''])[0]
            if new_ip:
                save_known_ip(new_ip)
            self._send_json({"ip": get_known_ip()})
            return

        # 5. Firebase configuration proxy
        if path == '/api/esp32/firebase-config':
            target_ip = query.get('ip', [get_known_ip()])[0]
            host = query.get('host', [''])[0]
            auth = query.get('auth', [''])[0]
            clear = query.get('clear', [''])[0]
            try:
                p_dict = {}
                if host:
                    p_dict['host'] = host
                if auth:
                    p_dict['auth'] = auth
                if clear:
                    p_dict['clear'] = '1'
                params = urllib.parse.urlencode(p_dict)
                req = urllib.request.Request(f"http://{target_ip}/firebase/config?{params}", headers={'User-Agent': 'SmartPlugApp/1.0', 'Connection': 'close'})
                with urllib.request.urlopen(req, timeout=4.0) as resp:
                    data = json.loads(resp.read().decode('utf-8'))
                    self._send_json(data)
                    return
            except Exception as e:
                self._send_json({"success": False, "error": str(e)}, status=500)
                return

        # 6. Reset on/off stats proxy
        if path == '/api/esp32/stats-reset':
            target_ip = query.get('ip', [get_known_ip()])[0]
            try:
                req = urllib.request.Request(f"http://{target_ip}/stats/reset")
                with urllib.request.urlopen(req, timeout=3.0) as resp:
                    data = json.loads(resp.read().decode('utf-8'))
                    self._send_json(data)
                    return
            except Exception as e:
                self._send_json({"success": False, "error": str(e)}, status=500)
                return

        # 7. Unpair & Factory Reset Proxy
        if path == '/api/esp32/unpair' or path == '/api/esp32/reset':
            target_ip = query.get('ip', [get_known_ip()])[0]
            try:
                if os.path.exists(IP_CACHE_FILE):
                    try:
                        os.remove(IP_CACHE_FILE)
                    except Exception:
                        pass
                req = urllib.request.Request(f"http://{target_ip}/unpair", headers={'User-Agent': 'SmartPlugApp/1.0', 'Connection': 'close'})
                with urllib.request.urlopen(req, timeout=2.5) as resp:
                    data = json.loads(resp.read().decode('utf-8'))
                    self._send_json(data)
                    return
            except Exception as e:
                self._send_json({"success": True, "note": f"Reset dispatched ({e})"}, status=200)
                return

        # 8. Clear IP Cache endpoint
        if path == '/api/esp32/clear-cache':
            if os.path.exists(IP_CACHE_FILE):
                try:
                    os.remove(IP_CACHE_FILE)
                except Exception:
                    pass
            self._send_json({"cleared": True})
            return

        # Serve static files with no-cache headers for dev
        return super().do_GET()

    def _send_json(self, payload, status=200):
        data = json.dumps(payload).encode('utf-8')
        self.send_response(status)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def end_headers(self):
        if self.path.endswith('.html') or self.path == '/' or self.path.endswith('sw.js') or self.path.endswith('.json'):
            self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
        super().end_headers()

if __name__ == '__main__':
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), PWAHandler) as httpd:
        print(f"My Smart Plug Server running at http://localhost:{PORT}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass
