import json
from collections import defaultdict
from PIL import Image, ImageDraw, ImageFont

# === CONFIGURATION ===
INPUT_JSON = "tilemap_out.json"
INPUT_IMAGE = "C:\\Users\\Skyri\\CLionProjects\\Game\\resources\\textures\\level2.png"
OUTPUT_BASE = "overlay_layer"

TILE_WIDTH = 16
TILE_HEIGHT = 16
TEXTURE_WIDTH = 128
TEXTURE_HEIGHT = 432
SCALE = 2

tiles_per_row = TEXTURE_WIDTH // TILE_WIDTH

# === Load JSON data ===
with open(INPUT_JSON, "r") as f:
    data = json.load(f)

# === Load tileset image ===
tileset_image = Image.open(INPUT_IMAGE).convert("RGBA")

# === Load font ===
try:
    font = ImageFont.truetype("PressStart2P.ttf", 20)
except:
    font = ImageFont.load_default()

def draw_text_with_outline(draw, position, text, font, fill, outline):
    x, y = position
    for dx in [-1, 0, 1]:
        for dy in [-1, 0, 1]:
            if dx != 0 or dy != 0:
                draw.text((x + dx, y + dy), text, font=font, fill=outline)
    draw.text(position, text, font=font, fill=fill)

def render_layer(layer_data, layer_index):
    max_x = max_y = 0
    for y, row in enumerate(layer_data):
        max_x = max(max_x, len(row) - 1)
        max_y = max(max_y, y)

    output_width = (max_x + 1) * TILE_WIDTH * SCALE
    output_height = (max_y + 1) * TILE_HEIGHT * SCALE
    image = Image.new("RGBA", (output_width, output_height))
    draw = ImageDraw.Draw(image)

    for y, row in enumerate(layer_data):
        for x, tile_id in enumerate(row):
            if tile_id == -1:
                continue

            tx = (tile_id % tiles_per_row) * TILE_WIDTH
            ty = (tile_id // tiles_per_row) * TILE_HEIGHT
            dest_x = x * TILE_WIDTH * SCALE
            dest_y = y * TILE_HEIGHT * SCALE

            tile = tileset_image.crop((tx, ty, tx + TILE_WIDTH, ty + TILE_HEIGHT))
            tile = tile.resize((TILE_WIDTH * SCALE, TILE_HEIGHT * SCALE), Image.NEAREST)
            image.paste(tile, (dest_x, dest_y), tile)

            label = str(tile_id)
            bbox = draw.textbbox((0, 0), label, font=font)
            text_w = bbox[2] - bbox[0]
            text_h = bbox[3] - bbox[1]
            px = dest_x + (TILE_WIDTH * SCALE - text_w) // 2
            py = dest_y + (TILE_HEIGHT * SCALE - text_h) // 2
            draw_text_with_outline(draw, (px, py), label, font, fill=(255, 255, 255), outline=(0, 0, 0))

    image.save(f"{OUTPUT_BASE}{layer_index}.png")
    print(f"✅ Saved: {OUTPUT_BASE}{layer_index}.png")

# === Render each layer separately ===
if "tileLayers" in data:
    for index, layer in enumerate(data["tileLayers"]):
        render_layer(layer["tilemap"], index)
else:
    raise RuntimeError("❌ Expected 'tileLayers' format in input JSON.")
