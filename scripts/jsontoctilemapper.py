import json
from collections import defaultdict

# === CONFIGURATION ===
INPUT_FILE = "input.json"
OUTPUT_FILE = "tilemap_out.json"

TILE_WIDTH = 16
TILE_HEIGHT = 16
TEXTURE_WIDTH = 224
TEXTURE_HEIGHT = 240
tiles_per_row = TEXTURE_WIDTH // TILE_WIDTH

# === Load JSON ===
with open(INPUT_FILE, "r") as f:
    data = json.load(f)

# === Auto-detect input format ===
if "tiles" in data:
    print("🔍 Detected flat Pixlab export format.")
    tiles = data["tiles"]
elif "tilesetEditing" in data:
    try:
        tiles = data["tilesetEditing"][0]["layers"][0]["tiles"]
        print("🔍 Detected nested Pixlab project format.")
    except (KeyError, IndexError):
        raise RuntimeError("⚠️ Invalid nested tilesetEditing structure.")
else:
    raise RuntimeError("❌ Unknown input format. Could not locate tile definitions.")

# === Build 2D tilemap array ===
grid = defaultdict(dict)
max_x = max_y = 0

for key, tile in tiles.items():
    if not key.strip():
        continue
    try:
        gx, gy = map(int, key.split('-'))
        tile_id = tile["y"] * tiles_per_row + tile["x"]
        grid[gy][gx] = tile_id
        max_x = max(max_x, gx)
        max_y = max(max_y, gy)
    except ValueError:
        print(f"⚠️ Skipping invalid tile key: {key}")

tilemap_array = []
for y in range(max_y + 1):
    row = [grid[y].get(x, -1) for x in range(max_x + 1)]
    tilemap_array.append(row)

# === Write JSON file in clean style ===
with open(OUTPUT_FILE, "w") as out:
    out.write('{\n  "tilemap": [\n')
    for i, row in enumerate(tilemap_array):
        row_str = ', '.join(str(val) for val in row)
        comma = ',' if i < len(tilemap_array) - 1 else ''
        out.write(f"    [{row_str}]{comma}\n")
    out.write("  ]\n}\n")

print(f"✅ Clean JSON written to: {OUTPUT_FILE}")
