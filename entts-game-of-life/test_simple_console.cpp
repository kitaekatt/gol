#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Simple test - type 'q' and press ENTER to quit\n";
    
    bool running = true;
    while (running) {
        std::cout << "Frame update...\n";
        
        // Check for input (non-blocking would be better but this is a test)
        if (std::cin.rdbuf()->in_avail() > 0) {
            char c;
            std::cin >> c;
            std::cout << "Got: " << c << std::endl;
            if (c == 'q') {
                std::cout << "Quitting!\n";
                running = false;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    return 0;
}