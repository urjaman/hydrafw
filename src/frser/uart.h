#pragma once

uint8_t frser_uart_recv(void);
void frser_uart_send(uint8_t val);

/* Fake. USB 1.1 as the UART. */
#define BAUD 8000000

#define RECEIVE() frser_uart_recv()
#define SEND(n) frser_uart_send(n)
#define UART_BUFLEN 4096
#define UARTTX_BUFLEN 0
