import json
from collections import defaultdict
from PIL import Image, ImageDraw, ImageFont

# === CONFIGURATION ===
INPUT_JSON = "input.json"
INPUT_IMAGE = "C:\\Users\\Skyri\\CLionProjects\\Game\\resources\\textures\\punyworld-overworld-tileset.png"
OUTPUT_IMAGE = "overlay_output.png"

TILE_WIDTH = 16
TILE_HEIGHT = 16
TEXTURE_WIDTH = 224
TEXTURE_HEIGHT = 240
SCALE = 2

tiles_per_row = TEXTURE_WIDTH // TILE_WIDTH

# === Load JSON data ===
with open(INPUT_JSON, "r") as f:
    data = json.load(f)

# === Auto-detect tiles ===
tiles = {}

if "tiles" in data:
    print("🔍 Detected flat Pixlab format.")
    tiles = data["tiles"]

elif "tilesetEditing" in data:
    try:
        raw_tiles = data["tilesetEditing"][0]["layers"][0]["tiles"]
        print("🔍 Detected nested Pixlab project format.")
        tiles = {f"{tile['x']}-{tile['y']}": {"x": tile["x"], "y": tile["y"]} for tile in raw_tiles}
    except (KeyError, IndexError):
        raise RuntimeError("⚠️ Could not locate tile data in tilesetEditing structure.")

elif "layers" in data:
    try:
        raw_tiles = data["layers"][0]["tiles"]
        print("🔍 Detected custom layer format.")
        tiles = {f"{tile['x']}-{tile['y']}": {"id": int(tile["id"]), "x": tile["x"], "y": tile["y"]} for tile in raw_tiles}
    except (KeyError, IndexError):
        raise RuntimeError("⚠️ Could not parse layer format.")

else:
    raise RuntimeError("❌ Unknown tilemap format.")

# === Build tile grid ===
grid = defaultdict(dict)
max_x = max_y = 0

for key, tile in tiles.items():
    if not key.strip():
        continue
    try:
        gx, gy = map(int, key.split('-'))
        if "id" in tile:
            tile_id = tile["id"]
        else:
            tile_id = tile["y"] * tiles_per_row + tile["x"]
        grid[gy][gx] = tile_id
        max_x = max(max_x, gx)
        max_y = max(max_y, gy)
    except ValueError:
        print(f"⚠️ Skipping invalid tile key: {key}")

# === Set up output canvas ===
output_width = (max_x + 1) * TILE_WIDTH
output_height = (max_y + 1) * TILE_HEIGHT

tileset_image = Image.open(INPUT_IMAGE).convert("RGBA")
overlay_image = Image.new("RGBA", (output_width * SCALE, output_height * SCALE))
draw = ImageDraw.Draw(overlay_image)

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

# === Render tile overlay ===
for y in range(max_y + 1):
    for x in range(max_x + 1):
        tile_id = grid[y].get(x, -1)
        if tile_id == -1:
            continue

        tx = (tile_id % tiles_per_row) * TILE_WIDTH
        ty = (tile_id // tiles_per_row) * TILE_HEIGHT

        dest_x = x * TILE_WIDTH * SCALE
        dest_y = y * TILE_HEIGHT * SCALE

        tile_region = tileset_image.crop((tx, ty, tx + TILE_WIDTH, ty + TILE_HEIGHT))
        tile_region = tile_region.resize((TILE_WIDTH * SCALE, TILE_HEIGHT * SCALE), Image.NEAREST)
        overlay_image.paste(tile_region, (dest_x, dest_y))

        # Draw tile ID
        label = str(tile_id)
        bbox = draw.textbbox((0, 0), label, font=font)
        text_w = bbox[2] - bbox[0]
        text_h = bbox[3] - bbox[1]
        px = dest_x + (TILE_WIDTH * SCALE - text_w) // 2
        py = dest_y + (TILE_HEIGHT * SCALE - text_h) // 2
        draw_text_with_outline(draw, (px, py), label, font, fill=(255, 255, 255), outline=(0, 0, 0))

# === Save result ===
overlay_image.save(OUTPUT_IMAGE)
print(f"✅ Overlay image saved to {OUTPUT_IMAGE}")
