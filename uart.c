/*
 * UART
 */

#include <unistd.h>
#include <malloc.h>
#include <pthread.h>

#include "device.h"
#include "util.h"

#define UART_ADDRESS_SPACE_START 0x0000000010000000
#define UART_ADDRESS_SPACE_END   0x00000000100000FF

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

#define UART_LSR_TEMT 0x40  /* Transmitter empty */
#define UART_LSR_THRE 0x20  /* Transmit-hold-register empty */
#define UART_LSR_BI   0x10  /* Break interrupt indicator */
#define UART_LSR_FE   0x08  /* Frame error indicator */
#define UART_LSR_PE   0x04  /* Parity error indicator */
#define UART_LSR_OE   0x02  /* Overrun error indicator */
#define UART_LSR_DR   0x01  /* Receiver data ready */

#define UART_LCR_DLAB 0x80  /* DLAB enable */

#define UART_IER_MSI    0x08    /* Enable Modem status interrupt */
#define UART_IER_RLSI   0x04    /* Enable receiver line status interrupt */
#define UART_IER_THRI   0x02    /* Enable Transmitter holding register int. */
#define UART_IER_RDI    0x01    /* Enable receiver data interrupt */

#define UART_IIR_MSI    0x00    /* Modem status interrupt */
#define UART_IIR_THRI   0x02    /* Transmitter holding register empty */
#define UART_IIR_RDI    0x04    /* Receiver data interrupt */
#define UART_IIR_RLSI   0x06    /* Receiver line status interrupt */

#define UART_IIR_NO_INT 0x01    /* No interrupts pending */
#define UART_IIR_ID     0x06    /* Mask for the interrupt ID */


typedef struct _uart_t
{
    device_t dev;

    uint32_t irq_num;

    uint8_t rbr;
    uint8_t ier;
    uint8_t iir;
    uint8_t lcr;
    uint8_t mcr;
    uint8_t lsr;
    uint8_t msr;
    uint8_t scr;

    uint16_t divider;

    bool    thr_pending;
} uart_t;


static uint64_t
uart_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint8_t ret;
    uart_t *uart = (uart_t *) dev;

    switch (addr)
    {
    case UART_RBR:  /* 0 */
        uart->lsr &= ~(UART_LSR_DR | UART_LSR_BI);

        if (uart->ier & UART_IER_THRI)
            uart->iir = UART_IIR_THRI;
        else
            uart->iir = UART_IIR_NO_INT;

        if (uart->iir != UART_IIR_NO_INT)
            plic_signal(uart->irq_num);

        return uart->rbr;

    case UART_IER:  /* 1 */
        return uart->ier;

    case UART_IIR:  /* 2 */
        ret = uart->iir;
        if ((ret & UART_IIR_ID) == UART_IIR_THRI) {
            uart->thr_pending = false;
            uart->iir = UART_IIR_NO_INT;
        }
        return ret;

    case UART_LCR:  /* 3 */
        return uart->lcr;

    case UART_LSR:  /* 5 */
        return uart->lsr;

    case UART_MSR:  /* 6 */
        return uart->msr;

    case UART_SCR:  /* 7 */
        return uart->scr;

    default:
        panic("%s: [0x%lx]\n", __func__, addr);
    }

    return 0;
}

static uint64_t
uart_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    uart_t *uart = (uart_t *) dev;

    if (size != 1)
        panic("%s: bad size %d\n", __func__, size);

    switch (addr)
    {
    case UART_THR:  /* 0 */
        if (uart->lcr & UART_LCR_DLAB) {
            uart->divider = (uart->divider & 0xFF00) | (data & 0xFF);
        } else {
            uart->thr_pending = false;
            putchar((uint8_t)data);
            fflush(stdout);
        }
        break;

    case UART_IER:  /* 1 */
        if (uart->lcr & UART_LCR_DLAB) {
            uart->divider = ((data & 0xFF) << 8) | (uart->divider & 0x00FF);
        } else {
            bool changed = (uart->ier ^ data) & 0x0f;
            uart->ier = (uint8_t)data;
            if (changed) {
                if (uart->ier & UART_IER_THRI)
                    uart->thr_pending = true;

                if (uart->ier & UART_IER_RLSI)
                    uart->iir = UART_IIR_RLSI;
                else if ((uart->ier & UART_IER_RDI) &&
                         (uart->lsr & UART_LSR_DR))
                    uart->iir = UART_IIR_RDI;
                else if ((uart->ier & UART_IER_THRI) && uart->thr_pending)
                    uart->iir = UART_IIR_THRI;
                else if (uart->ier & UART_IER_MSI)
                    uart->iir = UART_IIR_MSI;
                else
                    uart->iir = UART_IIR_NO_INT;

                if (uart->iir != UART_IIR_NO_INT)
                    plic_signal(uart->irq_num);
            }
        }
        break;

    case UART_IIR:  /* 2 */
        uart->iir = (uint8_t)data;
        break;

    case UART_LCR:  /* 3 */
        uart->lcr = (uint8_t)data;
        break;

    case UART_MCR:  /* 4 */
        uart->mcr = (uint8_t)data;
        break;

    case UART_MSR:  /* 6 */
        uart->msr = (uint8_t)data;
        break;

    case UART_SCR:  /* 7 */
        uart->scr = (uint8_t)data;
        break;

    default:
        panic("%s: [0x%lx]: 0x%lx\n", __func__, addr, data);
    }

    return 0;
}

static void *
_routine(void *arg)
{
    uart_t *uart = (uart_t *) arg;

    while (1) {
        uart->rbr = getch();
        if (uart->rbr == 3) /* CTRL_C */
            break;

        uart->lsr |= UART_LSR_DR;
        if (uart->ier & UART_IER_RDI) {
            uart->iir = UART_IIR_RDI;
            plic_signal(uart->irq_num);
        }
    }

    return NULL;
}

device_t *
uart_init(address_space *parent_as, uint32_t irq_num)
{
    pthread_t tid;
    uart_t *uart;

    uart = calloc(1, sizeof(uart_t));
    uart->dev.name = "uart";

    uart->irq_num = irq_num;

    uart->lsr = UART_LSR_THRE | UART_LSR_TEMT;

    init_address_space(&(uart->dev.as),
                       UART_ADDRESS_SPACE_START,
                       UART_ADDRESS_SPACE_END);

    uart->dev.as.ops.read_op = uart_read;
    uart->dev.as.ops.write_op = uart_write;

    uart->dev.as.device = uart;

    register_address_space(parent_as, &(uart->dev.as));

    pthread_create(&tid, NULL, _routine, uart);

    return (device_t *) uart;
}
