/*
 * serial_lora_sender.cpp
 * YOLO renk tespiti → COM port → LoRa gönderici
 *
 * Kırmızı → '0'  |  Siyah → '1'  |  Yeşil → '2'
 *
 * Derleme (MSVC) : cl /EHsc serial_lora_sender.cpp
 * Derleme (MinGW): g++ -o serial_lora_sender.exe serial_lora_sender.cpp
 */

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstdint>
#include <cstdio>

// ── Yapılandırma ────────────────────────────────
#define COM_PORT   "\\\\.\\COM3"   // COM10+ için \\.\COM10 formatı zorunlu
#define BAUD_RATE  CBR_9600
// ────────────────────────────────────────────────

enum class Color : uint8_t { Kirmizi = 0, Siyah = 1, Yesil = 2, Unknown = 0xFF };

// ────────────────────────────────────────────────
static HANDLE portOpen(const char* name)
{
    HANDLE h = CreateFileA(name, GENERIC_READ | GENERIC_WRITE,
                           0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return h;

    DCB dcb       = {};
    dcb.DCBlength = sizeof(DCB);
    GetCommState(h, &dcb);
    dcb.BaudRate  = BAUD_RATE;
    dcb.ByteSize  = 8;
    dcb.StopBits  = ONESTOPBIT;
    dcb.Parity    = NOPARITY;
    dcb.fOutxCtsFlow = dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl  = DTR_CONTROL_DISABLE;
    dcb.fRtsControl  = RTS_CONTROL_DISABLE;
    dcb.fOutX = dcb.fInX = FALSE;
    SetCommState(h, &dcb);

    COMMTIMEOUTS to = { 50, 10, 100, 0, 0 };
    SetCommTimeouts(h, &to);

    return h;
}

// ────────────────────────────────────────────────
// YOLO string çıktısını Color enum'a çevirir.
// İlk karaktere bakarak O(1) karşılaştırma yapar.
static Color parseColor(const char* s)
{
    switch (s[0]) {
        case 'k': case 'K': return Color::Kirmizi; // "kırmızı"
        case 's': case 'S': return Color::Siyah;   // "siyah"
        case 'y': case 'Y': return Color::Yesil;   // "yeşil"
        default:  return Color::Unknown;
    }
}

// ────────────────────────────────────────────────
// Color → ASCII ID gönderir.
// Integer değil '0'/'1'/'2' karakteri yazılır → LoRa'da bozuk karakter olmaz.
static bool sendColor(HANDLE h, Color c)
{
    char id;
    switch (c) {
        case Color::Kirmizi: id = '0'; break;
        case Color::Siyah:   id = '1'; break;
        case Color::Yesil:   id = '2'; break;
        default:             return false;
    }

    const char buf[2] = { id, '\n' };
    DWORD written;
    return WriteFile(h, buf, 2, &written, nullptr) && (written == 2);
}

// ────────────────────────────────────────────────
int main()
{
    HANDLE h = portOpen(COM_PORT);
    if (h == INVALID_HANDLE_VALUE) {
        printf("[ERR] Port acilamadi: %s\n", COM_PORT);
        return 1;
    }
    printf("[OK] Port hazir: %s | 9600 baud | 8N1\n\n", COM_PORT);

    char input[16];
    while (scanf("%15s", input) == 1) {
        if (input[0] == 'q') break;

        Color c = parseColor(input);

        if (c == Color::Unknown) { puts("[!] Bilinmeyen renk"); continue; }

        printf(sendColor(h, c) ? "[TX] Gonderildi\n" : "[ERR] Yazma hatasi\n");
    }

    CloseHandle(h);
    return 0;
}