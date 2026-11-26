#include "rgb_sensor.h"
#include "tm4c1294ncpdt.h"
#include <stdio.h>

static RgbReading latest;
static char line[128];
static size_t line_length;

void rgb_sensor_init(void)
{
    latest = (RgbReading){0};
    line_length = 0;

    // enable GPIO Port P and UART6
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R13;
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R6;

    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R13) == 0U) {}
    while ((SYSCTL_PRUART_R & SYSCTL_PRUART_R6) == 0U) {}

    // connect UART6 to Port P pins 0 and 1
    UART6_CTL_R = 0U;
    GPIO_PORTP_AMSEL_R &= ~0x01U;
    GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & ~0x0FU) | 0x01U;
    GPIO_PORTP_AFSEL_R |= 0x01U;
    GPIO_PORTP_DEN_R |= 0x01U;

    // 16 MHz alternate clock source
    SYSCTL_ALTCLKCFG_R = (SYSCTL_ALTCLKCFG_R & ~(uint32_t)SYSCTL_ALTCLKCFG_ALTCLK_M) | SYSCTL_ALTCLKCFG_ALTCLK_PIOSC;
    UART6_CC_R = UART_CC_CS_PIOSC;

    // 1200 baud, 8 data bits, no parity, 1 stop bit, FIFO enabled
    UART6_IBRD_R = 833U;
    UART6_FBRD_R = 21U;
    UART6_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;
    UART6_CTL_R = UART_CTL_RXE | UART_CTL_UARTEN;
}

void rgb_sensor_poll(void)
{
    while ((UART6_FR_R & UART_FR_RXFE) == 0U) {
        char ch = (char)(UART6_DR_R & 0xFFU);

        if (ch == '\n') {
            unsigned int r, g, b;
            line[line_length] = '\0';
            line_length = 0;

            if (sscanf(line, "R = %u | G = %u | B = %u", &r, &g, &b) == 3) {
                latest = (RgbReading){(uint8_t)r, (uint8_t)g, (uint8_t)b, true};
                return;  // One complete RGB reading per poll
            }
        } 
        else if (line_length < sizeof(line) - 1U) {
            line[line_length++] = ch;
        }
    }
}

RgbReading rgb_sensor_get_latest(void){
    return latest;
}