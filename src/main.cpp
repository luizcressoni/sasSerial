#include <iostream>
#include <unistd.h>

#include "serialportsas.h"
#include <iomanip>

int main(int argc, char *argv[]){
    std::cout << "SAS Library test\n";
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    cSerialPortSas serialport(argv[1]);

    if (!serialport.open()) {
        std::cout << "Error opening serial port\n";
        return 1;
    }
    for (int i=0;i<5;i++) {
        serialport.putChar(0x01, true);
        std::cout << "Chirping 0x01...\n";
        sleep(1);
    }

    uint8_t byte;
    for (int i=0;i<1000;) {
        uint8_t aux = serialport.getChar(&byte);
        if (aux & UART_HAS_RX) {
            std::cout << "0x"
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(byte)
              << ((aux & WAKEUP_BIT) ? "!" : " ")
              << std::endl;
            i++;
        }
    }

    serialport.close();
    std::cout << "Bye!\n";
    return 0;
}