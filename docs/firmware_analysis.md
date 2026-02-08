# Phân tích chi tiết Firmware: full_flash.bin

> **Ngày phân tích:** 08/02/2026  
> **File:** `full_flash.bin`  
> **Kích thước:** 4,194,304 bytes (4 MB)  
> **MD5:** `324CD547C2B125863C0C2E8A2B2B18E7`

---

## 1. Tổng quan

File `full_flash.bin` là một bản dump **toàn bộ flash 4MB** từ một thiết bị **ESP8266 (ESP-07S)** chạy firmware ứng dụng **IOT E-Ink / E-Paper** dùng cho hệ thống **gọi y tá (Call Nurse)** trong môi trường bệnh viện. Thiết bị hiển thị thông tin trên màn hình e-paper, kết nối WiFi để nhận lệnh từ server, hỗ trợ OTA update, và có chế độ tiết kiệm pin (deep sleep).

---

## 2. Thông tin phần cứng & nền tảng

| Thông số | Giá trị |
|---|---|
| **MCU** | ESP8266 (ESP-07S) |
| **Flash size** | 4 MB |
| **Flash mode** | DIO |
| **Flash speed** | 40 MHz |
| **Framework** | Arduino (ESP8266 Arduino Core) |
| **SDK Version** | 2.5.0 |
| **BearSSL** | Build 6778687 |
| **Filesystem** | SPIFFS |
| **Display** | E-Paper / E-Ink (BMP bitmap, 1-bit depth) |

---

## 3. Bản đồ bộ nhớ Flash (Memory Map)

```
┌─────────────────────────────────────────────────────────────┐
│ Offset          │ Kích thước │ Nội dung                     │
├─────────────────┼────────────┼──────────────────────────────┤
│ 0x000000        │ 4 KB       │ Bootloader (1st stage)       │
│ 0x001000        │ 501.6 KB   │ Application Image (chính)    │
│ 0x07E674        │ ~5.4 KB    │ Padding / Empty              │
│ 0x080000        │ 4 KB       │ Empty (sector gap)           │
│ 0x081000        │ 4 KB       │ Bootloader (backup copy)     │
│ 0x082000        │ ~501 KB    │ Application (backup copy)    │
│ 0x100000        │ ~16 KB     │ SPIFFS Filesystem            │
│ 0x104000        │ ~3036 KB   │ Empty (SPIFFS free space)    │
│ 0x3FB000        │ 4 KB       │ RF Calibration Data          │
│ 0x3FC000        │ 4 KB       │ ESP8266 PHY Init Data        │
│ 0x3FD000        │ 4 KB       │ System Parameters (slot 1)   │
│ 0x3FE000        │ 4 KB       │ System Parameters (slot 2)   │
│ 0x3FF000        │ 4 KB       │ System Parameters (slot 3)   │
└─────────────────┴────────────┴──────────────────────────────┘
```

> **Lưu ý:** Flash chứa **2 bản copy** giống nhau của bootloader + application (tại 0x000000 và 0x081000), đây là cơ chế **dual-partition** để đảm bảo an toàn khi OTA update.

---

## 4. Phân tích Bootloader (0x000000)

| Trường | Giá trị |
|---|---|
| **Magic byte** | `0xE9` (hợp lệ ESP8266) |
| **Số segment** | 1 |
| **Flash mode** | DIO (0x02) |
| **Flash size** | 4MB |
| **Entry point** | `0x4010F29C` |
| **Segment 0** | Addr: `0x4010F000`, Size: 1,384 bytes |

---

## 5. Phân tích Application Image (0x001000)

| Trường | Giá trị |
|---|---|
| **Magic byte** | `0xE9` |
| **Số segment** | 4 |
| **Flash mode** | DIO |
| **Entry point** | `0x401000E0` |
| **Tổng kích thước** | 513,652 bytes (~501.6 KB) |
| **MD5 (app)** | `E42B89B9D49E97CC50FD532817F89A4F` |

### Bảng Segments

| Segment | Địa chỉ bắt đầu | Địa chỉ kết thúc | Kích thước | Mô tả |
|---|---|---|---|---|
| 0 | `0x40201010` | `0x40272ED8` | 455.7 KB | **IROM** - Code chính (flash-mapped) |
| 1 | `0x40100000` | `0x40106EC0` | 27.7 KB | **IRAM** - Code thời gian thực (cache) |
| 2 | `0x3FFE8000` | `0x3FFE8590` | 1.4 KB | **DRAM** - Initialized data (.data) |
| 3 | `0x3FFE85A0` | `0x3FFEC8D4` | 16.8 KB | **DRAM** - Initialized data (.rodata) |

---

## 6. Hệ thống file SPIFFS (0x100000)

SPIFFS filesystem chứa **9 file** cấu hình và dữ liệu cache:

| File | Mô tả |
|---|---|
| `/formated.txt` | Flag đánh dấu SPIFFS đã được format |
| `/working` | Flag trạng thái hoạt động |
| `/wifi.txt` | Cấu hình WiFi (SSID, password, server, IP...) |
| `/token` | Token xác thực với server |
| `/deliverm` | Bitmap BMP cho chế độ "Deliver" (giao hàng) |
| `/maininfo1` | Bitmap cache cho màn hình chính (page 1) |
| `/maininfo2` | Bitmap cache cho màn hình chính (page 2) |
| `/maininfo3` | Bitmap cache cho màn hình chính (page 3) |
| `/lowpower` | Bitmap BMP hiển thị khi pin yếu |

### Nội dung file `/wifi.txt` (đã giải mã)

```
mode:     dhcp
ssid:     zyc
password: 12345678
server:   47.94.90.7:71
ip:       -- (DHCP)
mask:     -- (DHCP)
gw:       -- (DHCP)
phy:      N (802.11n)
adc:      333
did:      bed
```

### Bitmap trong SPIFFS

File `/deliverm` và `/lowpower` chứa ảnh **BMP 1-bit** (đen trắng) dùng cho e-paper:
- `/deliverm`: BMP header cho thấy kích thước **71 x 16 pixels** (dạng icon nhỏ)
- `/lowpower`: BMP header cho thấy kích thước **508 x 24 pixels** (banner cảnh báo pin yếu)

---

## 7. Phân tích chức năng ứng dụng

### 7.1. Mục đích chính
Thiết bị là một **màn hình e-paper IoT trong bệnh viện** với chức năng:
- **Hiển thị thông tin giường bệnh** (thông tin bệnh nhân)
- **Nút gọi y tá** ("Call Nurse") - gửi yêu cầu lên server
- **Cập nhật nội dung hiển thị** từ server qua WiFi (bitmap BMP)
- **OTA firmware update** từ xa
- **Chế độ tiết kiệm pin** (deep sleep)

### 7.2. Chế độ hoạt động

| Chế độ | Mô tả |
|---|---|
| **Work Mode** | Chế độ làm việc bình thường - poll server để nhận bitmap mới |
| **Deliver Mode** | Chế độ giao hàng/vận chuyển - hiển thị màn hình tĩnh |
| **Setting Mode** | Chế độ cài đặt WiFi và cấu hình |
| **Low Power** | Hiển thị cảnh báo pin yếu và vào deep sleep |

### 7.3. API Endpoints (giao tiếp với server)

| Endpoint | Mô tả |
|---|---|
| `GET /api/token?mac=` | Lấy token xác thực bằng MAC address |
| `GET /api/call?token=` | Gửi lệnh gọi y tá |
| `GET /api/pollb?token=&force=&clientver=&battery=&gap=&rssi=&ssid=&ip=&deviceid=&wifi=` | Poll server để nhận bitmap cập nhật |
| `GET /api/ota?token=&model=&version=` | Kiểm tra & tải firmware OTA update |
| `GET /test.bmp` | Tải bitmap test (factory mode) |

### 7.4. Luồng hoạt động chính (Work Mode)

```
┌──────────────┐
│   Wake up     │ (từ deep sleep hoặc reset)
└──────┬───────┘
       ▼
┌──────────────┐
│  Init SPIFFS  │ → Format nếu lần đầu
└──────┬───────┘
       ▼
┌──────────────┐
│ Đọc wifi.txt  │ → Cấu hình WiFi (DHCP/Static)
└──────┬───────┘
       ▼
┌──────────────┐
│ Kết nối WiFi  │ → Timeout → Hiển thị lỗi
└──────┬───────┘
       ▼
┌──────────────┐
│ Lấy Token     │ → /api/token?mac=...
└──────┬───────┘
       ▼
┌──────────────┐
│ Poll Server   │ → /api/pollb?token=...
└──────┬───────┘
       ▼
┌──────────────────┐
│ Nhận & hiển thị   │ → Decode BMP → Ghi SPIFFS cache
│ bitmap e-paper    │    → Refresh màn hình
└──────┬───────────┘
       ▼
┌──────────────────┐
│ Kiểm tra OTA      │ → /api/ota?token=...
└──────┬───────────┘
       ▼
┌──────────────┐
│ Deep Sleep    │ → Ngủ N giây, rồi lặp lại
└──────────────┘
```

### 7.5. Menu cài đặt (Function Button)

Người dùng truy cập menu bằng các nút vật lý **K1** và **K2**:

```
<1> Switch Mode:   Nhấn K2 x 1 lần  → Chuyển Work/Deliver mode
<2> Setup WiFi:    Nhấn K2 x 3 lần  → Cài đặt WiFi
<3> RESET:         Nhấn K2 x 9 lần  → Factory reset
    Auto Exit:     Tự thoát sau 120 giây
    Press K1:      Thoát ngay lập tức
```

### 7.6. Các lệnh FLAG (Server → Device)

Server có thể gửi các flag điều khiển thiết bị:

| Flag | Chức năng |
|---|---|
| `work mode` | Chuyển sang chế độ làm việc |
| `switch mode` | Chuyển đổi giữa Work/Deliver |
| `setting wifi` | Kích hoạt cài đặt WiFi |
| `ota` | Kích hoạt OTA update |
| `factory reset` | Reset về cài đặt gốc |
| `force update` | Buộc cập nhật bitmap |
| `emergency call` | Gọi y tá khẩn cấp |

---

## 8. Cấu hình chân GPIO (Pin Configuration)

### 8.1. Phương pháp phân tích

Cấu hình chân được xác định bằng các phương pháp:
- Phân tích các thanh ghi ngoại vi (peripheral registers) được tham chiếu trong firmware
- Trích xuất bảng GPIO MUX từ phân đoạn `.data` (3FFE8560)
- Phân tích dữ liệu waveform LUT của e-paper controller
- Đối chiếu chuỗi debug và chức năng phần cứng
- Suy luận từ thiết kế phần cứng ESP8266 tiêu chuẩn

> ⚠️ **Lưu ý:** Firmware đã được biên dịch (compiled Xtensa binary) và **không** chứa chuỗi gán chân rõ ràng (ví dụ: "DC=4", "CS=15"). Các chân EPD DC, RST, BUSY và nút bấm được suy luận từ thiết kế phần cứng phổ biến, chân đã xác nhận được đánh dấu ✅, chân suy luận được đánh dấu ⚡.

### 8.2. Bảng cấu hình chân

| Chức năng | GPIO | Trạng thái | Bằng chứng |
|---|---|---|---|
| **SPI MOSI** (E-Paper DIN) | GPIO13 | ✅ Xác nhận | Thanh ghi HSPI (0x60000100-0x600001FC) trong firmware |
| **SPI CLK** (E-Paper CLK) | GPIO14 | ✅ Xác nhận | HSPI cố định trên ESP8266 |
| **SPI CS** (E-Paper CS) | GPIO15 | ✅ Xác nhận | HSPI CS0 mặc định trên ESP8266 |
| **EPD DC** | GPIO4 | ⚡ Suy luận | Chân phổ biến nhất cho DC trên thiết kế ZRWL |
| **EPD RST** | GPIO2 | ⚡ Suy luận | Chân phổ biến cho RST (có pull-up nội bộ) |
| **EPD BUSY** | GPIO5 | ⚡ Suy luận | Chuỗi "Busy Timeout!" xác nhận có sử dụng BUSY pin |
| **Nút K1** (Exit) | GPIO0 | ⚡ Suy luận | GPIO0 thường dùng làm nút FLASH/boot kiêm nút user |
| **Nút K2** (Function) | GPIO12 | ⚡ Suy luận | GPIO khả dụng, phổ biến cho nút thứ 2 |
| **Deep Sleep Wake** | GPIO16 → RST | ✅ Xác nhận | Chuỗi "DEEP_SLEEP_AWAKE" - bắt buộc cho ESP8266 deep sleep |
| **ADC** (Đo pin) | A0 | ✅ Xác nhận | Chuỗi "adc value=%d", cấu hình "adc=333" trong wifi.txt |

### 8.3. Chi tiết phân tích

#### SPI (HSPI) — ✅ Xác nhận
Firmware sử dụng **HSPI** (Hardware SPI) của ESP8266, xác nhận qua các thanh ghi:
```
0x60000100 - SPI_CMD (SPI Command Register)
0x60000108 - SPI_ADDR
0x6000010C - SPI_CTRL
0x60000118 - SPI_CLK (SPI Clock Register)  
0x6000011C - SPI_USER
0x60000120 - SPI_USER1
0x6000012C - SPI_USER2
0x60000140 - SPI_W0 (SPI Data Buffer W0)
0x600001FC - SPI_EXT3
```

ESP8266 HSPI sử dụng chân cố định:
- **GPIO12** = MISO (không dùng cho e-paper 1 chiều)
- **GPIO13** = MOSI (→ EPD DIN)
- **GPIO14** = SCK (→ EPD CLK)
- **GPIO15** = CS0 (→ EPD CS)

#### GPIO — ✅ Xác nhận có sử dụng
Các thanh ghi GPIO được tham chiếu:
```
0x60000310 - GPIO_ENABLE_W1TS (GPIO enable set)
0x60000314 - GPIO_ENABLE_W1TC (GPIO enable clear)
0x60000318 - GPIO_IN (GPIO input value)
```

#### E-Paper Controller — ✅ Xác nhận SSD1675-style
Phân đoạn `.data` tại địa chỉ **3FFE8020** chứa **~640 bytes** dữ liệu waveform LUT đặc trưng của controller **SSD1675/IL3820**:
```
3FFE8020: 3A 01 02 01 DF 00 D5 00 3A 01 A2 00 7F 00 75 00
3FFE8030: ... (tiếp tục với pattern 0x02, 0x05, 0x0B - voltage levels)
```

Firmware có các hàm điều khiển EPD:
- `_PowerOn` — Bật nguồn e-paper
- `_PowerOff` — Tắt nguồn e-paper
- `_Update_Full` — Refresh toàn bộ màn hình
- `_Update_Part` — Refresh một phần màn hình (partial update)

#### Deep Sleep — ✅ Xác nhận
ESP8266 deep sleep yêu cầu **GPIO16 nối với RST** qua điện trở. Chuỗi `DEEP_SLEEP_AWAKE` và `EXT_SYS_RST` xác nhận thiết bị sử dụng deep sleep với wake-up qua timer RTC.

#### ADC — ✅ Xác nhận
- Chân **A0** (ADC duy nhất trên ESP8266) dùng đo điện áp pin
- Chuỗi debug: `adc value=%d`
- Ngưỡng trong cấu hình: `adc=333` (trong wifi.txt)
- Khi pin yếu: hiển thị bitmap `/lowpower` và vào deep sleep sâu

#### Nút bấm — Xác nhận có 2 nút
Firmware có 2 nút vật lý (từ chuỗi debug):
- **K1**: Nút thoát (`Press K1 to exit at once!`)
- **K2**: Nút chức năng (`setting--function button pressed:%d`)
- Hàm khởi tạo: `setting_init_function_button()`

### 8.4. Bảng GPIO MUX (từ .data segment)

Bảng ánh xạ GPIO → IO_MUX Register Offset (tiêu chuẩn ESP8266 Arduino Core), tìm thấy tại **3FFE8560**:

```
GPIO0  → MUX offset 0x34    GPIO8  → MUX offset 0x24
GPIO1  → MUX offset 0x18    GPIO9  → MUX offset 0x28
GPIO2  → MUX offset 0x38    GPIO10 → MUX offset 0x2C
GPIO3  → MUX offset 0x14    GPIO11 → MUX offset 0x30
GPIO4  → MUX offset 0x3C    GPIO12 → MUX offset 0x04
GPIO5  → MUX offset 0x40    GPIO13 → MUX offset 0x08
GPIO6  → MUX offset 0x1C    GPIO14 → MUX offset 0x0C
GPIO7  → MUX offset 0x20    GPIO15 → MUX offset 0x10
```

### 8.5. Sơ đồ kết nối (suy luận)

```
ESP8266 (ESP-07S)          E-Paper Display (SSD1675)
┌──────────────┐           ┌──────────────┐
│         GPIO13├──────────►│DIN (MOSI)    │  ✅
│         GPIO14├──────────►│CLK (SCK)     │  ✅
│         GPIO15├──────────►│CS             │  ✅
│          GPIO4├──────────►│DC             │  ⚡
│          GPIO2├──────────►│RST            │  ⚡
│          GPIO5│◄──────────┤BUSY           │  ⚡
│              │           └──────────────┘
│              │           
│          GPIO0│◄─── Nút K1 (Exit)         ⚡
│         GPIO12│◄─── Nút K2 (Function)     ⚡
│              │           
│            A0│◄─── Đo điện áp pin (ADC)  ✅
│              │           
│         GPIO16├───┐                        ✅
│           RST│◄──┘ (Deep Sleep Wake)
└──────────────┘
```

### 8.6. Thông tin bổ sung từ firmware

| Thông tin | Giá trị |
|---|---|
| **Developer** | `hansong` (macOS: `/Users/hansong/Library/Arduino15/...`) |
| **Build tool** | Arduino IDE + ESP8266 Core 2.5.0 |
| **EPD driver** | Custom (không phải GxEPD, Adafruit, hay Waveshare library) |
| **E-paper LUT** | Waveform data cho SSD1675/IL3820 (full + partial refresh) |
| **Flash chip** | GD25Q32C (GigaDevice 32Mbit/4MB) |

> 💡 **Để xác định chính xác cấu hình chân**, cần sử dụng **Xtensa disassembler** (Ghidra với xtensa plugin hoặc IDA Pro) để phân tích mã máy. Các chân được đánh dấu ⚡ có thể khác với thực tế.

---

## 9. Bảo mật

### 9.1. Mã hóa AES
Firmware sử dụng mã hóa AES cho giao tiếp:

| Thông số | Giá trị |
|---|---|
| **AES IV** | Được tạo từ chuỗi cấu hình |
| **AES Key** | Được tạo từ chuỗi cấu hình |
| **Chuỗi nhận dạng** | `IOTEInk8` |
| **Key liên quan 1** | `ExbL380QoGzLTZYv` (16 bytes - AES-128) |
| **Key liên quan 2** | `LTZgjumb80h6HRod` (16 bytes - AES-128) |

### 9.2. WiFi đã lưu trong System Parameters

| Thông số | Giá trị |
|---|---|
| **SSID** | `zyc` |
| **Password** | `12345678` |

### 9.3. Factory defaults

| Thông số | Giá trị |
|---|---|
| **Factory SSID** | `zrwl` |
| **Factory Password** | `zrwl1234` |
| **Factory Server** | `47.94.90.7:71` |

---

## 10. Thông tin hiển thị (E-Paper)

### Trang thông tin trạng thái thiết bị hiển thị:

| Trường | Mô tả |
|---|---|
| `Server:` | Địa chỉ server |
| `ADC:` | Giá trị ADC (đo pin) |
| `WiFi PHY Mode:` | Chế độ WiFi (B/G/N) |
| `WiFi RSSI:` | Cường độ tín hiệu |
| `VERSION:` | Phiên bản firmware |
| `UUID:` | ID thiết bị |
| `GAP:` | Khoảng thời gian poll (giây) |
| `Current Mode:` | Work / Deliver |
| `Current Version:` | Phiên bản hiện tại |
| `Last update:` | Thời gian cập nhật cuối |

### Xử lý ảnh BMP
- Firmware download ảnh BMP từ server
- Hỗ trợ **1-bit BMP** (đen trắng, phù hợp e-paper)
- Parse: File size → Image Offset → Header size → Bit Depth → Image size
- Cache 3 trang bitmap vào SPIFFS (`/maininfo1`, `/maininfo2`, `/maininfo3`)
- Hiển thị bitmap cache khi không có kết nối mạng

---

## 11. PHY Init & RF Calibration

### PHY Init Data (0x3FC000)
```
00 00 FF 00 4C 1D 4B 23 4D 24 4D 23 00 00 00 00
```
- Cấu hình RF chuẩn cho ESP8266
- TX power levels và frequency calibration

### System Parameters (0x3FD000 - 0x3FF000)
- 3 slot lưu trữ xoay vòng (wear leveling)
- Chứa WiFi SSID/password đã kết nối
- Cấu hình boot mode

---

## 12. Thư viện sử dụng

Dựa trên phân tích chuỗi trong firmware:

| Thư viện | Mục đích |
|---|---|
| **ESP8266WiFi** | Kết nối WiFi |
| **ESP8266HTTPClient** | HTTP client giao tiếp với server |
| **SPIFFS** | Hệ thống file flash |
| **BearSSL** | Hỗ trợ SSL/TLS |
| **Arduino Core 2.5.0** | Framework chính |

---

## 13. Tóm tắt kỹ thuật

```
╔══════════════════════════════════════════════════════════════╗
║  Thiết bị: ESP-07S E-Paper IoT (Call Nurse System)         ║
║  Hãng phát triển: ZRWL (推测 - dựa trên Factory SSID)       ║
║  Tên sản phẩm: IOTEInk8                                    ║
║  Server: 47.94.90.7:71 (Alibaba Cloud - China)             ║
║  Flash: 4MB DIO 40MHz                                      ║
║  App size: ~502 KB (chiếm ~12% flash)                      ║
║  SPIFFS: ~3 MB (đa phần trống)                             ║
║  Chức năng: Hiển thị thông tin giường bệnh + Gọi y tá     ║
║  Giao tiếp: HTTP + AES-128 encryption                      ║
║  Tiết kiệm pin: Deep sleep giữa các lần poll              ║
║  Cập nhật: OTA update qua WiFi                             ║
╚══════════════════════════════════════════════════════════════╝
```

---

## 14. Hướng dẫn flash lại firmware

### Flash toàn bộ (full flash dump):
```bash
esptool.py --port COMx --baud 460800 write_flash 0x0 full_flash.bin
```

### Flash chỉ application:
```bash
# Extract app từ full_flash.bin (offset 0x1000, size ~502KB)
esptool.py --port COMx --baud 460800 write_flash 0x1000 app.bin
```

### Xóa flash trước khi ghi:
```bash
esptool.py --port COMx erase_flash
esptool.py --port COMx --baud 460800 write_flash 0x0 full_flash.bin
```

---

## 15. Lưu ý bảo mật

⚠️ File firmware chứa các thông tin nhạy cảm:
- **WiFi credentials:** SSID `zyc`, password `12345678`
- **Factory WiFi:** SSID `zrwl`, password `zrwl1234`  
- **Server address:** `47.94.90.7:71`
- **AES keys:** `ExbL380QoGzLTZYv`, `LTZgjumb80h6HRod`

Không nên chia sẻ file firmware này công khai nếu các thông tin trên vẫn còn sử dụng.
