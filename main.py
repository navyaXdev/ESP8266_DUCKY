from flask import Flask, request, Response
import pyautogui
import time
import threading
import psutil
import pygetwindow as gw
from collections import deque
import re

app = Flask(__name__)

# Configuration
pyautogui.FAILSAFE = True
pyautogui.PAUSE = 0.01

# Thread-safe deque for tracking apps
last_opened_apps = deque(maxlen=10)
window_lock = threading.Lock()

@app.after_request
def after_request(response):
    """Enable CORS"""
    response.headers.add('Access-Control-Allow-Origin', '*')
    response.headers.add('Access-Control-Allow-Headers', 'Content-Type')
    response.headers.add('Access-Control-Allow-Methods', 'POST')
    return response

def track_new_windows():
    """Background thread to monitor newly opened windows"""
    known_windows = set()
    while True:
        try:
            with window_lock:
                current_windows = set(w.title for w in gw.getAllWindows() if w.title)
                new_windows = current_windows - known_windows
                if new_windows:
                    for title in new_windows:
                        if title.strip():  # Skip empty titles
                            last_opened_apps.appendleft(title)
                    known_windows = current_windows
        except Exception as e:
            print(f"Window tracking error: {e}")
        time.sleep(1)

def activate_window(window_title, timeout=3):
    """Activate window by title with verification"""
    try:
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                with window_lock:
                    win = gw.getWindowsWithTitle(window_title)[0]
                    win.activate()
                    time.sleep(0.3)
                    if win.isActive:
                        return True
            except IndexError:
                time.sleep(0.3)
    except Exception as e:
        print(f"Window activation failed: {e}")
    return False

def focus_last_software():
    """Focus on the most recently opened software"""
    try:
        with window_lock:
            if not last_opened_apps:
                print("No recently opened apps in history")
                return False
            
            # Create a copy of the deque for safe iteration
            apps_to_try = list(last_opened_apps)
        
        for title in apps_to_try:
            if activate_window(title):
                print(f"Focused on: {title}")
                return True
        return False
    except Exception as e:
        print(f"Error focusing last software: {e}")
        return False

def type_text_safely(text, char_delay=0.05):
    """Type text with error handling"""
    for char in text:
        try:
            pyautogui.write(char)
            time.sleep(char_delay)
        except Exception as e:
            print(f"Failed to type character: {e}")
            continue

def execute_command_sequence(payload):
    """Process the command payload"""
    try:
        # Normalize input
        payload = re.sub(r'\r\n', '\n', payload)
        lines = [line.strip() for line in payload.split('\n') if line.strip()]
        
        i = 0
        while i < len(lines):
            line = lines[i]
            
            if line == 'WINR':
                pyautogui.hotkey('win', 'r')
                time.sleep(0.5)
                i += 1
                
            elif line == 'SOFTWARE':
                if not focus_last_software():
                    print("Failed to focus last software")
                i += 1
                
            elif line.startswith('TYPE:'):
                # Get text to type (multi-line support)
                text_content = line[5:]
                i += 1
                while i < len(lines) and not lines[i].startswith(('WINR', 'SOFTWARE', 'TYPE:', 'ENTER', 'TAB', 'ESC', 'DELAY:', 'HOTKEY:')):
                    text_content += '\n' + lines[i]
                    i += 1
                
                # Type the text
                type_text_safely(text_content)
                
            elif line == 'ENTER':
                pyautogui.press('enter')
                i += 1
                
            elif line == 'TAB':
                pyautogui.press('tab')
                i += 1
                
            elif line == 'ESC':
                pyautogui.press('esc')
                i += 1
                
            elif line.startswith('DELAY:'):
                delay = float(line[6:]) / 1000.0
                time.sleep(max(delay, 0.1))
                i += 1
                
            elif line.startswith('HOTKEY:'):
                keys = [k.strip().lower() for k in line[7:].split(',')]
                pyautogui.hotkey(*keys)
                i += 1
                
            else:
                i += 1  # Skip unrecognized commands
                
            time.sleep(0.1)  # Small delay between commands
            
    except Exception as e:
        print(f"Command execution error: {e}")

@app.route('/inject', methods=['POST', 'OPTIONS'])
def handle_injection():
    """Endpoint for receiving payloads"""
    if request.method == 'OPTIONS':
        return Response(status=200)
    
    try:
        payload = request.data.decode('utf-8')
        print(f"Received payload ({len(payload)} chars)")
        
        # Start execution in background thread
        thread = threading.Thread(target=execute_command_sequence, args=(payload,))
        thread.start()
        
        return Response("Command processing started", status=200)
    except Exception as e:
        return Response(f"Error: {str(e)}", status=500)

if __name__ == '__main__':
    # Start window tracking thread
    tracker_thread = threading.Thread(target=track_new_windows, daemon=True)
    tracker_thread.start()
    
    print("Starting HID Controller on port 5000")
    print("Tracking window openings in background...")
    app.run(host='0.0.0.0', port=5000, threaded=True)