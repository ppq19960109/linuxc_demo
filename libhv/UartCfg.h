#ifndef __UART_CFG_H_
#define __UART_CFG_H_
enum UART_BAUDRATE
{
    BAUDRATE_4800 = 4800,
    BAUDRATE_9600 = 9600,
    BAUDRATE_19200 = 19200,
    BAUDRATE_57600 = 57600,
    BAUDRATE_115200 = 115200,
    BAUDRATE_460800 = 460800,
};

enum UART_DATABIT
{
    DATABIT_5 = 5,
    DATABIT_6,
    DATABIT_7,
    DATABIT_8,
};

enum UART_PARITY
{
    PARITY_NONE = 'N',
    PARITY_ODD = 'O',
    PARITY_EVEN = 'E',
};
enum UART_STOPBIT
{
    STOPBIT_1 = 1,
    STOPBIT_2 = 2,
};
enum UART_FLOWCTRL
{
    FLOWCTRL_NONE = 0,
    FLOWCTRL_HARDWARE,
    FLOWCTRL_SOFTWARE,
};

enum UART_BLOCKING
{
    BLOCKING_BLOCK = 0,
    BLOCKING_NONBLOCK,
};

int uart_init(const char *device, enum UART_BAUDRATE baudrate, enum UART_DATABIT databit, enum UART_PARITY parity, enum UART_STOPBIT stopbit, enum UART_FLOWCTRL flowCtrl, enum UART_BLOCKING blocking);
#endif
