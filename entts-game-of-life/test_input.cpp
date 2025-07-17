#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

bool kbhit() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    struct timeval timeout = {0, 0}; // No wait
    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) > 0;
}

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    std::cout << "Input test - press 'q' to quit, any other key to test\n";
    enableRawMode();
    
    while (true) {
        if (kbhit()) {
            char c = getchar();
            std::cout << "Got char: '" << c << "' (code: " << static_cast<int>(c) << ")\n";
            if (c == 'q') {
                std::cout << "Quitting!\n";
                break;
            }
        }
        usleep(50000); // 50ms
    }
    
    return 0;
}