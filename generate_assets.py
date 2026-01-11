import os

import re

# Paths
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, 'data')
OUTPUT_FILE = os.path.join(BASE_DIR, 'src', 'WebAssets.h')

def minify_html(content):
    # Remove HTML comments
    content = re.sub(r'<!--.*?-->', '', content, flags=re.DOTALL)
    # Remove whitespace between tags (simple approach)
    content = re.sub(r'>\s+<', '><', content)
    return content.strip()

def minify_css(content):
    # Remove comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove whitespace
    content = re.sub(r'\s*([:;{},])\s*', r'\1', content)
    content = re.sub(r'\s+', ' ', content)
    return content.strip()

def minify_js(content):
    # Remove multi-line comments first
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove single line comments but NOT URLs (http://, https://)
    # Only remove // that are preceded by whitespace or start of line
    content = re.sub(r'(^|\s)//.*', r'\1', content, flags=re.MULTILINE)
    # Remove unnecessary whitespace (basic) but preserve : in URLs
    # Don't remove spaces around : to avoid breaking "http :" -> "http:"
    content = re.sub(r'\s*([=+\-*/{}()[\\],;])\s*', r'\1', content)
    content = re.sub(r'\s+', ' ', content)
    return content.strip()

def generate_assets():
    print("🚀 Generating & Minifying WebAssets.h from data folder...")
    
    try:
        # Read files
        with open(os.path.join(DATA_DIR, 'index.html'), 'r', encoding='utf-8') as f:
            html = minify_html(f.read())
        with open(os.path.join(DATA_DIR, 'style.css'), 'r', encoding='utf-8') as f:
            css = minify_css(f.read())
        with open(os.path.join(DATA_DIR, 'script.js'), 'r', encoding='utf-8') as f:
            js = minify_js(f.read())

        # Inline CSS
        css_tag = f"<style>{css}</style>"
        html = html.replace('<link rel="stylesheet" href="style.css" />', css_tag)
        
        # Inline JS
        js_tag = f"<script>{js}</script>"
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
