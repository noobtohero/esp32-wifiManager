import os

# Paths
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, 'data')
OUTPUT_FILE = os.path.join(BASE_DIR, 'lib', 'WiFiManager', 'src', 'WebAssets.h')

def generate_assets():
    print("🚀 Generating WebAssets.h from data folder...")
    
    try:
        # Read files
        with open(os.path.join(DATA_DIR, 'index.html'), 'r', encoding='utf-8') as f:
            html = f.read()
        with open(os.path.join(DATA_DIR, 'style.css'), 'r', encoding='utf-8') as f:
            css = f.read()
        with open(os.path.join(DATA_DIR, 'script.js'), 'r', encoding='utf-8') as f:
            js = f.read()

        # Inline CSS
        css_tag = f"<style>\n{css}\n</style>"
        html = html.replace('<link rel="stylesheet" href="style.css" />', css_tag)
        
        # Inline JS
        js_tag = f"<script>\n{js}\n</script>"
        html = html.replace('<script src="script.js"></script>', js_tag)

        # Header Template
        header_content = f"""#ifndef WM_WEB_ASSETS_H
#define WM_WEB_ASSETS_H

#include <Arduino.h>

const char WM_HTML_INDEX[] PROGMEM = R"rawliteral(
{html}
)rawliteral";

#endif
"""

        # Write output
        with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
            f.write(header_content)
            
        print(f"✅ Successfully generated: {OUTPUT_FILE}")
        print(f"📦 Total size in Flash: {len(header_content) / 1024:.2f} KB")

    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == "__main__":
    generate_assets()
