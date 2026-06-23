#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>

// Pi'de USB TTL kullanıyorsan genelde ttyUSB0'dır. GPIO pinlerini kullanıyorsan serial0 olur.
#define COM_PORT "/dev/ttyUSB0" 
#define BAUD_RATE B9600

enum class Color : uint8_t { Kirmizi = 0, Siyah = 1, Yesil = 2, Unknown = 0xFF };

static int portOpen(const char* name) {
    int fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) return -1;

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) return -1;

    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 bit
    tty.c_cflag &= ~PARENB;                     // No parity
    tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                    // Akış kontrolü kapalı
    
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;                        // 0.5 saniye timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tcsetattr(fd, TCSANOW, &tty);
    return fd;
}

static Color parseColor(const char* s) {
    switch (s[0]) {
        case 'k': case 'K': return Color::Kirmizi;
        case 's': case 'S': return Color::Siyah;
        case 'y': case 'Y': return Color::Yesil;
        default:  return Color::Unknown;
    }
}

static bool sendColor(int fd, Color c) {
    char id;
    switch (c) {
        case Color::Kirmizi: id = '0'; break;
        case Color::Siyah:   id = '1'; break;
        case Color::Yesil:   id = '2'; break;
        default:             return false;
    }

    const char buf[2] = { id, '\n' };
    return write(fd, buf, 2) == 2;
}

int main() {
    int fd = portOpen(COM_PORT);
    if (fd < 0) {
        printf("[ERR] Port acilamadi: %s. 'sudo usermod -a -G dialout $USER' yapip yeniden baslattigindan emin ol.\n", COM_PORT);
        return 1;
    }
    printf("[OK] Port hazir: %s | 9600 baud | 8N1\n\n", COM_PORT);

    char input[16];
    while (scanf("%15s", input) == 1) {
        if (input[0] == 'q') break;

        Color c = parseColor(input);

        if (c == Color::Unknown) { puts("[!] Bilinmeyen renk"); continue; }

        printf(sendColor(fd, c) ? "[TX] Gonderildi\n" : "[ERR] Yazma hatasi\n");
    }

    close(fd);
    return 0;
}
