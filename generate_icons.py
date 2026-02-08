
def print_c_array(name, grid):
    output = []
    output.append(f"const unsigned char {name}[] PROGMEM = {{")
    hex_vals = []
    
    for y in range(64):
        row_bytes = []
        current_byte = 0
        for x in range(64):
            pixel = 1 if grid[y][x] else 0
            # MSB First
            bit_pos = 7 - (x % 8)
            if pixel:
                current_byte |= (1 << bit_pos)
            
            if (x % 8) == 7:
                row_bytes.append(f"0x{current_byte:02x}")
                current_byte = 0
        hex_vals.append(", ".join(row_bytes))
    
    output.append(",\n".join(hex_vals))
    output.append("};\n")
    return "\n".join(output)
    lines.append("};\n")
    return "\n".join(lines)

def dist_sq(x1, y1, x2, y2):
    return (x1-x2)**2 + (y1-y2)**2

output_content = ""
output_content += "#ifndef WEATHER_ICONS_H\n"
output_content += "#define WEATHER_ICONS_H\n"
output_content += "#include <Arduino.h>\n"

# 1. SUN (Solid Circle + Rays)
grid_sun = [[False]*64 for _ in range(64)]
for y in range(64):
    for x in range(64):
        # Body
        if dist_sq(x, y, 32, 32) <= 16**2:
            grid_sun[y][x] = True
        # Rays
        if 28 <= y <= 36 and (x < 12 or x > 52):
             grid_sun[y][x] = True
        if 28 <= x <= 36 and (y < 12 or y > 52):
             grid_sun[y][x] = True

# 2. CLOUD (Solid)
grid_cloud = [[False]*64 for _ in range(64)]
for y in range(64):
    for x in range(64):
        # Left Lump
        if dist_sq(x, y, 20, 38) <= 14**2:
            grid_cloud[y][x] = True
        # Right Lump
        if dist_sq(x, y, 44, 38) <= 14**2:
            grid_cloud[y][x] = True
        # Top Lump
        if dist_sq(x, y, 32, 28) <= 16**2:
            grid_cloud[y][x] = True
        # Flatten bottom
        if grid_cloud[y][x] and y > 50:
             grid_cloud[y][x] = False
        # Fill bottom rect
        if 20 < x < 44 and 38 < y < 50:
            grid_cloud[y][x] = True

# 3. RAIN (Cloud + Drops)
grid_rain = [row[:] for row in grid_cloud] # Copy cloud
for y in range(52, 62):
    for x in range(64):
        # Drop 1
        if 25 <= x <= 28: grid_rain[y][x] = True
        # Drop 2
        if 35 <= x <= 38: grid_rain[y][x] = True
        # Drop 3
        if 45 <= x <= 48: grid_rain[y][x] = True

output_content += print_c_array("icon_sun_64", grid_sun)
output_content += print_c_array("icon_cloud_64", grid_cloud)
output_content += print_c_array("icon_rain_64", grid_rain)
output_content += "#endif\n"

with open("include/weather_icons.h", "w", encoding="utf-8") as f:
    f.write(output_content)
