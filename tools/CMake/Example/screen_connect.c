#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "lib/rs232/rs232.h"

#define RTU_PORT        (10)
#define SCREEN_PORT     (12)

int32_t rtu_port_handle = RTU_PORT - 1;
int32_t screen_port_handle = SCREEN_PORT - 1;

void port_init(int32_t port, uint32_t baud) {
    char mode[] = { '8', 'N', '1', 0 };

    if (RS232_OpenComport(port, baud, mode)) {
        printf("Can not open comport\n");

        exit(0);
    }
}

void print_hexstring(unsigned char *buf, uint32_t len)
{
    for(uint32_t i = 0; i < len; ++i) {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

int32_t get_port_data(int32_t port_handle, unsigned char *buf, int32_t delay_ms)
{
    uint16_t i = 0;
    while(delay_ms-- > 0) {
        if(RS232_PollComport(port_handle, &buf[i], 1) <= 0) {
            delay_ms--;
            Sleep(1);
        } else {
            i++;
        }
    }

    return i;
}

int main(void)
{
    unsigned char buf[1024] = {0};

    port_init(rtu_port_handle, 2400);
    port_init(screen_port_handle, 2400);

    while(true) {
        memset(buf, 0, 1024);
        int32_t n = get_port_data(rtu_port_handle, buf, (int32_t)250);
        if(n > 0) {
            print_hexstring(buf, n);
            RS232_SendBuf(screen_port_handle, buf, n);
            memset(buf, 0, 1024);
            Sleep(500);
            n = get_port_data(screen_port_handle, buf, (int32_t)250);
            if(n > 0) {
                print_hexstring(buf, n);
                RS232_SendBuf(rtu_port_handle, buf, n);
            }
        }
        Sleep(100);
    }
}
