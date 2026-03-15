"""
Download the complete Direct3D 11 documentation from Microsoft Learn.
Saves each page as a markdown file in docs/dx11_docs/.
"""
import os
import re
import time
import urllib.request
import urllib.error
from html.parser import HTMLParser

DOCS_DIR = os.path.join(os.path.dirname(__file__), "docs", "dx11_docs")
BASE_URL = "https://learn.microsoft.com/en-us/windows/win32/direct3d11/"

# All key DX11 documentation pages organized by section
PAGES = {
    # === Getting Started ===
    "00_overview": "atoc-dx-graphics-direct3d-11",
    "00_features": "direct3d-11-features",
    "00_how_to_use": "how-to-use-direct3d-11",
    "00_whats_new": "dx-graphics-overviews-introduction",
    
    # === Programming Guide ===
    "01_programming_guide": "dx-graphics-overviews",
    "01_migration_dx9_to_dx11": "d3d11-programming-guide-migrating",
    
    # === Devices ===
    "02_devices_overview": "overviews-direct3d-11-devices",
    "02_devices_intro": "overviews-direct3d-11-devices-intro",
    "02_devices_create": "overviews-direct3d-11-devices-create",
    "02_devices_create_swap_chain": "overviews-direct3d-11-devices-create-swap-chain",
    "02_devices_downlevel": "overviews-direct3d-11-devices-downlevel",
    "02_devices_downlevel_intro": "overviews-direct3d-11-devices-downlevel-intro",
    "02_devices_layers": "overviews-direct3d-11-devices-layers",
    
    # === Resources ===
    "03_resources_overview": "overviews-direct3d-11-resources",
    "03_resources_types": "overviews-direct3d-11-resources-types",
    "03_resources_buffers_intro": "overviews-direct3d-11-resources-buffers-intro",
    "03_resources_buffers_vb_how_to": "overviews-direct3d-11-resources-buffers-vertex-how-to",
    "03_resources_buffers_ib_how_to": "overviews-direct3d-11-resources-buffers-index-how-to",
    "03_resources_buffers_cb_how_to": "overviews-direct3d-11-resources-buffers-constant-how-to",
    "03_resources_textures": "overviews-direct3d-11-resources-textures",
    "03_resources_textures_intro": "overviews-direct3d-11-resources-textures-intro",
    "03_resources_textures_create": "overviews-direct3d-11-resources-textures-create",
    "03_resources_textures_how_to": "overviews-direct3d-11-resources-textures-how-to",
    "03_resources_textures_fill_manually": "overviews-direct3d-11-resources-textures-how-to-fill-manually",
    "03_resources_subresources": "overviews-direct3d-11-resources-subresources",
    "03_texture_block_compression": "texture-block-compression-in-direct3d-11",
    
    # === Graphics Pipeline ===
    "04_pipeline_overview": "overviews-direct3d-11-graphics-pipeline",
    "04_input_assembler": "d3d10-graphics-programming-guide-input-assembler-stage",
    "04_input_assembler_getting_started": "d3d10-graphics-programming-guide-input-assembler-stage-getting-started",
    "04_primitive_topologies": "d3d10-graphics-programming-guide-primitive-topologies",
    "04_vertex_shader": "vertex-shader-stage",
    "04_tessellation": "direct3d-11-advanced-stages-tessellation",
    "04_geometry_shader": "geometry-shader-stage",
    "04_stream_output": "d3d10-graphics-programming-guide-output-stream-stage",
    "04_rasterizer": "d3d10-graphics-programming-guide-rasterizer-stage",
    "04_pixel_shader": "pixel-shader-stage",
    "04_output_merger": "d3d10-graphics-programming-guide-output-merger-stage",
    "04_compute_shader": "direct3d-11-advanced-stages-compute-shader",
    
    # === Rendering ===
    "05_rendering_overview": "overviews-direct3d-11-render",
    "05_rendering_multi_thread": "overviews-direct3d-11-render-multi-thread",
    "05_rendering_multi_thread_intro": "overviews-direct3d-11-render-multi-thread-intro",
    "05_rendering_multi_thread_command_list": "overviews-direct3d-11-render-multi-thread-command-list",
    
    # === Effects ===
    "06_effects": "d3d11-graphics-programming-guide-effects",
    
    # === DX11.1 / DX11.2 Features ===
    "07_dx11_1_features": "direct3d-11-1-features",
    "07_dx11_2_features": "direct3d-11-2-features",
}


class SimpleHTMLToText(HTMLParser):
    """Minimal HTML to readable text converter."""
    def __init__(self):
        super().__init__()
        self.result = []
        self.skip = False
        self.in_pre = False
        self.in_code = False
        self.in_h = 0
        
    def handle_starttag(self, tag, attrs):
        attrs_dict = dict(attrs)
        if tag in ('script', 'style', 'nav', 'footer', 'header'):
            self.skip = True
        elif tag == 'pre':
            self.in_pre = True
            self.result.append('\n```\n')
        elif tag == 'code' and not self.in_pre:
            self.in_code = True
            self.result.append('`')
        elif tag in ('h1', 'h2', 'h3', 'h4', 'h5', 'h6'):
            self.in_h = int(tag[1])
            self.result.append('\n' + '#' * self.in_h + ' ')
        elif tag == 'p':
            self.result.append('\n\n')
        elif tag == 'br':
            self.result.append('\n')
        elif tag == 'li':
            self.result.append('\n- ')
        elif tag == 'a' and 'href' in attrs_dict:
            self.result.append('[')
        elif tag == 'table':
            self.result.append('\n')
        elif tag == 'tr':
            self.result.append('\n|')
        elif tag == 'th' or tag == 'td':
            self.result.append(' ')
            
    def handle_endtag(self, tag):
        if tag in ('script', 'style', 'nav', 'footer', 'header'):
            self.skip = False
        elif tag == 'pre':
            self.in_pre = False
            self.result.append('\n```\n')
        elif tag == 'code' and not self.in_pre:
            self.in_code = False
            self.result.append('`')
        elif tag in ('h1', 'h2', 'h3', 'h4', 'h5', 'h6'):
            self.in_h = 0
            self.result.append('\n')
        elif tag == 'th' or tag == 'td':
            self.result.append(' |')
            
    def handle_data(self, data):
        if self.skip:
            return
        if self.in_pre:
            self.result.append(data)
        else:
            self.result.append(data.strip() if not self.in_h else data)
    
    def get_text(self):
        return ''.join(self.result)


def download_page(slug, filename):
    """Download a single documentation page."""
    url = BASE_URL + slug
    try:
        req = urllib.request.Request(url, headers={
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) Metin2-DX11-Migration-Docs/1.0'
        })
        with urllib.request.urlopen(req, timeout=15) as resp:
            html = resp.read().decode('utf-8', errors='replace')
        
        # Extract main content area
        # Microsoft Learn uses <main> tag for content
        main_match = re.search(r'<main[^>]*>(.*?)</main>', html, re.DOTALL)
        if main_match:
            content_html = main_match.group(1)
        else:
            # Fallback: try article tag
            article_match = re.search(r'<article[^>]*>(.*?)</article>', html, re.DOTALL)
            content_html = article_match.group(1) if article_match else html
        
        # Convert HTML to text
        parser = SimpleHTMLToText()
        parser.feed(content_html)
        text = parser.get_text()
        
        # Clean up
        text = re.sub(r'\n{4,}', '\n\n\n', text)
        text = text.strip()
        
        # Add source header
        header = f"# {filename.replace('_', ' ').title()}\n\nSource: [{url}]({url})\n\n---\n\n"
        
        filepath = os.path.join(DOCS_DIR, f"{filename}.md")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(header + text)
        
        return True
    except Exception as e:
        print(f"  FAILED: {e}")
        return False


def main():
    os.makedirs(DOCS_DIR, exist_ok=True)
    
    total = len(PAGES)
    success = 0
    failed = 0
    
    print(f"Downloading {total} DX11 documentation pages...")
    print(f"Output: {DOCS_DIR}\n")
    
    for filename, slug in PAGES.items():
        print(f"  [{success+failed+1}/{total}] {filename}...", end=" ", flush=True)
        if download_page(slug, filename):
            print("OK")
            success += 1
        else:
            failed += 1
        time.sleep(0.3)  # Be polite to Microsoft servers
    
    print(f"\nDone! {success}/{total} pages downloaded ({failed} failed)")
    print(f"Location: {DOCS_DIR}")


if __name__ == "__main__":
    main()
