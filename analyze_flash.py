import re
import os

def extract_strings(data, min_len=4):
    result = ""
    for char in data:
        if 32 <= char <= 126:
            result += chr(char)
        else:
            if len(result) >= min_len:
                yield result
            result = ""
    if len(result) >= min_len:
        yield result

def analyze_binary(filepath):
    if not os.path.exists(filepath):
        print(f"File {filepath} not found.")
        return

    print(f"Analyzing {filepath}...")
    with open(filepath, "rb") as f:
        content = f.read()

    # 1. Strings Analysis
    print("\n--- String Analysis ---")
    strings = list(extract_strings(content))
    
    keywords_pins = ["GPIO", "Pin", "CS", "DC", "RST", "BUSY", "MOSI", "CLK", "MISO"]
    keywords_controllers = ["UC8179", "IL0373", "SSD1675", "SSD1680", "DEPG", "GDE", "UC8151", "UC8159"] 
    keywords_libs = ["GxEPD", "Adafruit", "U8g2", "ZinggJM"]

    found_pins = [s for s in strings if any(k in s for k in keywords_pins)]
    found_controllers = [s for s in strings if any(k in s for k in keywords_controllers)]
    found_libs = [s for s in strings if any(k in s for k in keywords_libs)]

    print(f"Found {len(strings)} total strings.")
    
    print("\n[Pin/Config Matches]")
    for s in found_pins[:20]: # Limit output
        print(s)
        
    print("\n[Controller Matches]")
    for s in found_controllers:
        print(s)

    print("\n[Library Matches]")
    for s in found_libs:
        print(s)

    # 2. Pattern Analysis
    print("\n--- Pattern Analysis ---")
    # T7 (UC8179) often uses 0x01 (Power Setting) followed by 4 bytes. 
    # But 0x01 is very common.
    # IL0373 often uses 0x01 (Gate Setting) followed by 3 bytes usually.
    
    # Let's look for "GxEPD" specific markers if defined in strings.
    
    # Generic search for sequence: CMD(0x01) DATA(0xXX, 0xXX, 0xXX, 0xXX) matches
    # This is tricky in raw binary without knowing the command structure (SPI vs I2C vs bitbang).
    # But if it's data array in memory:
    
    # Search for potential command tables.
    # 0x01 0x03 0x00 0x00 ... might be a power setting.
    
    # Let's just output if we see anything strongly resembling the text signatures first, 
    # as binary searching for 0x01 is too noisy without more constraints.
    pass

if __name__ == "__main__":
    analyze_binary("full_flash.bin")
