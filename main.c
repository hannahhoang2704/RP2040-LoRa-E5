//Implement a program that verifies that UART connection to LoRa module works and then reads and processes EUID from the
// module. The program should work as follows:
//1.Program waits for user to press SW_0. When user presses the button then program starts communication with the LoRa
// module.
//2.Program sends command “AT” to module and waits for response for 500 ms. If no response is received or the response
// is not correct the program tries again up to five times. If no response is received after five attempts program
// prints “module not responding” and goes back to step 1. If response is received program prints “Connected to LoRa
// module”.
//3.Program reads firmware version of the module and prints the result. If no response is received in 500 ms program
// prints “Module stopped responding” and goes back to step 1.
//4.Program reads DevEui from the device. If no response is received in 500 ms program prints “Module stopped
// responding” and goes back to step 1. DevEui contains 8 bytes that the module outputs in hexadecimal separated by
// colons. The program must remove the colons between the bytes and convert the hexadecimal digits to lower case.
//5.Go to step 1
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "uart.h"

#define STRLEN 80

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#if 0
#define UART_NR 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#else
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#endif

#define BAUD_RATE 9600
#define SW_0 9

bool pressed(uint button);
void modify_string(char *inputStr, char *changedStr);
int main() {

    gpio_init(SW_0);
    gpio_set_dir(SW_0, GPIO_IN);
    gpio_pull_up(SW_0);

    // Initialize chosen serial port
    stdio_init_all();

    uart_setup(UART_NR, UART_TX_PIN, UART_RX_PIN, BAUD_RATE);

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART

    const char command[] = "AT\r\n";
    const char command_version[] = "AT+VER\r\n";
    const char command_DevEui[] = "AT+ID=DevEui\r\n";
    char str[STRLEN];
    int pos = 0;
    bool button_pressed = false;
    bool read_version = false;
    bool read_DevEui = false;

    while (true) {

        if(!pressed(SW_0) && !button_pressed){
            printf("Button pressed\n");
            button_pressed = true;
            while(!gpio_get(SW_0)){
                sleep_ms(50);
            };
        }
        if(button_pressed) {
            uint count = 0;
            while (count < 5) {
                uart_send(UART_NR, command);
                sleep_ms(500);
                pos = uart_read(UART_NR, (uint8_t *) str, STRLEN);
                if (pos > 0) {
                    str[pos] = '\0';
                    printf("Connected to LoRa module\n");
                    printf("Command: %sReceived: %s\n", command, str);
                    pos = 0;
                    read_version = true;
                    break;
                }
                ++count;
            }
            if (count == 5) {
                printf("Module not responding\n");
                button_pressed = false;
            }
            if (read_version) {
                uart_send(UART_NR, command_version);
                sleep_ms(500);
                pos = uart_read(UART_NR, (uint8_t *) str, STRLEN);
                if (pos > 0) {
                    str[pos] = '\0';
                    printf("Command: %sReceived: %s\n", command_version, str);
                    pos = 0;
                    read_DevEui = true;
                    read_version = false;
                } else {
                    printf("Module not responding\n");
                    button_pressed = false;
                }
            }
            if (read_DevEui) {
                uart_send(UART_NR, command_DevEui);
                sleep_ms(500);
                pos = uart_read(UART_NR, (uint8_t *) str, STRLEN);
                if (pos > 0) {
                    str[pos] = '\0';
                    printf("Command: %sReceived: %s\n", command_DevEui, str);
                    char data_str[STRLEN];
                    char *devEuiStr = strstr(str, "DevEui,");
                    devEuiStr+=7;
                    modify_string(devEuiStr, data_str);
                    printf("%s\n\n", data_str);
                    pos = 0;
                    read_DevEui = false;
                }else{
                    printf("Module not responding\n");
                }
                button_pressed = false;
            }
        }
    }

}

bool pressed(uint button) {
    int press = 0;
    int release = 0;
    while (press < 3 && release < 3) {
        if (gpio_get(button)) {
            press++;
            release = 0;
        } else {
            release++;
            press = 0;
        }
        sleep_ms(10);
    }
    if (press > release) return true;
    else return false;
}

void modify_string(char *inputStr, char *changedStr){
    int inputStr_length = strlen(inputStr);
    int modifiedStr_index = 0;

    for(int i = 0; i < inputStr_length; ++i){
        if(isxdigit(inputStr[i])){
           changedStr[modifiedStr_index] = tolower(inputStr[i]);
           modifiedStr_index++;
        }
    }
    changedStr[modifiedStr_index] = '\0';
}