/*
 * UART
 */

#include <malloc.h>

#include "device.h"
#include "util.h"

#define UART_ADDRESS_SPACE_START 0x0000000010000000
#define UART_ADDRESS_SPACE_END   0x00000000100000FF

#define UART_LSR_THRE 0x20  /* Transmit-hold-register empty */
#define UART_LSR_TEMT 0x40  /* Transmitter empty */

#define UART_RBR 0      /* In:  Recieve Buffer Register */
#define UART_THR 0      /* Out: Transmitter Holding Register */
#define UART_DLL 0      /* Out: Divisor Latch Low */
#define UART_IER 1      /* I/O: Interrupt Enable Register */
#define UART_DLM 1      /* Out: Divisor Latch High */
#define UART_FCR 2      /* Out: FIFO Control Register */
#define UART_IIR 2      /* I/O: Interrupt Identification Register */
#define UART_LCR 3      /* Out: Line Control Register */
#define UART_MCR 4      /* Out: Modem Control Register */
#define UART_LSR 5      /* In:  Line Status Register */
#define UART_MSR 6      /* In:  Modem Status Register */
#define UART_SCR 7      /* I/O: Scratch Register */

typedef struct _uart_t
{
    device_t dev;

    uint8_t lsr;
} uart_t;


static uint64_t
uart_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uart_t *uart = (uart_t *) dev;

    if (addr == UART_LSR)
        return uart->lsr;

    return 0;
}

static uint64_t
uart_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    if (size != 1)
        panic("%s: bad size %d\n", __func__, size);

    if (addr == UART_THR)
        putchar((uint8_t)data);

    return 0;
}

device_t *
uart_init(address_space *parent_as)
{
    uart_t *uart;

    uart = calloc(1, sizeof(uart_t));
    uart->dev.name = "uart";

    uart->lsr = UART_LSR_THRE | UART_LSR_TEMT;

    init_address_space(&(uart->dev.as),
                       UART_ADDRESS_SPACE_START,
                       UART_ADDRESS_SPACE_END);

    uart->dev.as.ops.read_op = uart_read;
    uart->dev.as.ops.write_op = uart_write;

    uart->dev.as.device = uart;

    register_address_space(parent_as, &(uart->dev.as));

    return (device_t *) uart;
}
