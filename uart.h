//
// Created by Hanh Hoang on 6.11.2023.
//

#ifndef BLINK_UART_H
#define BLINK_UART_H

void uart_setup(int uart_nr, int tx_pin, int rx_pin, int speed);
int uart_read(int uart_nr, uint8_t *buffer, int size);
int uart_write(int uart_nr, const uint8_t *buffer, int size);
int uart_send(int uart_nr, const char *str);

#endif //BLINK_UART_H
