import json
from collections import defaultdict
"https://www.spritefusion.com/editor"
# === CONFIGURATION ===
INPUT_FILE = "input.json"
OUTPUT_FILE = "tilemap_out.json"

TILE_WIDTH = 16
TILE_HEIGHT = 16
TEXTURE_WIDTH = 128
TEXTURE_HEIGHT = 528
tiles_per_row = TEXTURE_WIDTH // TILE_WIDTH


# === Load JSON ===
with open(INPUT_FILE, "r") as f:
    data = json.load(f)

def build_tilemap_from_flat_dict(tiles_dict):
    grid = defaultdict(dict)
    max_x = max_y = 0

    for key, tile in tiles_dict.items():
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

    tilemap_array = []
    for y in range(max_y + 1):
        row = [grid[y].get(x, -1) for x in range(max_x + 1)]
        tilemap_array.append(row)

    return tilemap_array


def build_tilemap_from_layer(layer):
    grid = defaultdict(dict)
    max_x = max_y = 0
    for tile in layer.get("tiles", []):
        try:
            x, y = tile["x"], tile["y"]
            tile_id = int(tile["id"])
            grid[y][x] = tile_id
            max_x = max(max_x, x)
            max_y = max(max_y, y)
        except (KeyError, ValueError):
            continue

    tilemap_array = []
    for y in range(max_y + 1):
        row = [grid[y].get(x, -1) for x in range(max_x + 1)]
        tilemap_array.append(row)
    return tilemap_array


# === Auto-detect input format ===
output = {}

if "tiles" in data:
    print("🔍 Detected flat Pixlab export format.")
    tiles = data["tiles"]
    tiles = {f"{tile['x']}-{tile['y']}": tile for tile in tiles}
    output["tilemap"] = build_tilemap_from_flat_dict(tiles)

elif "tilesetEditing" in data:
    try:
        tiles = data["tilesetEditing"][0]["layers"][0]["tiles"]
        print("🔍 Detected nested Pixlab project format.")
        tiles = {f"{tile['x']}-{tile['y']}": tile for tile in tiles}
        output["tilemap"] = build_tilemap_from_flat_dict(tiles)
    except (KeyError, IndexError):
        raise RuntimeError("⚠️ Invalid nested tilesetEditing structure.")

elif "layers" in data:
    if len(data["layers"]) == 1:
        print("🔍 Detected single-layer format.")
        layer = data["layers"][0]
        tiles = {f"{tile['x']}-{tile['y']}": tile for tile in layer.get("tiles", [])}
        output["tilemap"] = build_tilemap_from_flat_dict(tiles)
    else:
        print("🔍 Detected multi-layer format.")
        output["tileLayers"] = []
        for layer in data["layers"]:
            tilemap = build_tilemap_from_layer(layer)
            output["tileLayers"].append({
                "name": layer.get("name", "Unnamed"),
                "tilemap": tilemap
            })
else:
    raise RuntimeError("❌ Unknown input format. Could not locate tile definitions.")

# === Write output JSON ===
# === Custom tilemap writer ===
def write_tilemap_json(data, path):
    def write_array(out, array, indent="    "):
        out.write("[\n")
        for i, row in enumerate(array):
            row_str = ", ".join(str(x) for x in row)
            comma = "," if i < len(array) - 1 else ""
            out.write(f"{indent}  [{row_str}]{comma}\n")
        out.write(f"{indent}]")

    with open(path, "w") as out:
        out.write("{\n")
        if "tilemap" in data:
            out.write('  "tilemap": ')
            write_array(out, data["tilemap"], indent="  ")
            out.write("\n")
        elif "tileLayers" in data:
            out.write('  "tileLayers": [\n')
            for i, layer in enumerate(data["tileLayers"]):
                comma = "," if i < len(data["tileLayers"]) - 1 else ""
                out.write("    {\n")
                out.write(f'      "name": "{layer["name"]}",\n')
                out.write(f'      "collidable": true,\n')
                out.write('      "tilemap": ')
                write_array(out, layer["tilemap"], indent="      ")
                out.write(f"\n    }}{comma}\n")
            out.write("  ]\n")
        out.write("}\n")

# === Write formatted output ===
write_tilemap_json(output, OUTPUT_FILE)
print(f"✅ Clean tilemap written to: {OUTPUT_FILE}")


print(f"✅ Output written to: {OUTPUT_FILE}")
