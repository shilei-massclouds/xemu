/*
 * CLINT
 */

#include <malloc.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "address_space.h"
#include "device.h"
#include "util.h"

#define CLINT_ADDRESS_SPACE_START 0x0000000002000000
#define CLINT_ADDRESS_SPACE_END   0x000000000200FFFF

#define CLINT_MSIP      0x0000
#define CLINT_MTIMECMP  0x4000
#define CLINT_MTIME     0xBFF8

static bool _software_intr;
static bool _timer_intr;

typedef struct _clint_t
{
    device_t dev;

    pthread_mutex_t _mutex;
    pthread_cond_t _cond;

    bool timer_running;

    uint64_t mtimecmp;
} clint_t;


intr_type_t
clint_interrupt(void)
{
    if (_timer_intr)
        return TIMER_INTR_TYPE;

    if (_software_intr)
        return SOFTWARE_INTR_TYPE;

    return INTR_TYPE_NONE;
}

static uint64_t
clint_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    switch (addr)
    {
    default:
        panic("%s: need to be implemented! [0x%lx]: (%u)\n",
              __func__, addr, size);
    }

    return 0;
}

static uint64_t
clint_write(void *dev, uint64_t addr, uint64_t data, size_t size,
            params_t params)
{
    clint_t *clint = (clint_t *) dev;

    switch (addr)
    {
    case CLINT_MSIP:
        _software_intr = (bool) data;
        printf("++++++++++++++++ Set msip\n");
        break;
    case CLINT_MTIMECMP:
        pthread_mutex_lock(&clint->_mutex);
        _timer_intr = false;
        clint->mtimecmp = data;

        if (cpu_read_rtc() > clint->mtimecmp) {
            /*
            printf("+++ +++ %s: Set true (%lx, %lx)\n",
                   __func__,
                   cpu_read_rtc(),
                   clint->mtimecmp);
                   */

            _timer_intr = true;
        } else {
            clint->timer_running = true;
        }
        pthread_mutex_unlock(&clint->_mutex);
        pthread_cond_signal(&clint->_cond);
        break;
    default:
        panic("%s: need to be implemented! [0x%lx]: 0x%lx (%u)\n",
              __func__, addr, data, size);
    }

    return 0;
}

static void *
_routine(void *arg)
{
    clint_t *clint = (clint_t *) arg;

    while (1) {
        pthread_mutex_lock(&clint->_mutex);

        while (!clint->timer_running) {
            pthread_cond_wait(&clint->_cond, &clint->_mutex);
        }

        while (cpu_read_rtc() < clint->mtimecmp) {
            struct timeval now;
            struct timespec next_time;
            gettimeofday(&now, NULL);
            next_time.tv_sec = now.tv_sec + 0;
            next_time.tv_nsec = (now.tv_usec + 1) * 1000;
            pthread_cond_timedwait(&clint->_cond, &clint->_mutex, &next_time);
            /*
            if (clint->mtimecmp != -1UL)
                printf("+++ +++ %s: (%lx, %lx, %lx)\n",
                       __func__,
                       current_ticks(),
                       clint->mtimecmp,
                       get_clock_realtime());
                       */
        }

        //printf("+++ +++ %s: Set true\n", __func__);
        _timer_intr = true;
        clint->timer_running = false;
        pthread_mutex_unlock(&clint->_mutex);
    }

    return NULL;
}

device_t *
clint_init(address_space *parent_as)
{
    pthread_t tid;
    clint_t *clint;

    clint = calloc(1, sizeof(clint_t));
    clint->dev.name = "clint";

    init_address_space(&(clint->dev.as),
                       CLINT_ADDRESS_SPACE_START,
                       CLINT_ADDRESS_SPACE_END);

    clint->dev.as.ops.read_op = clint_read;
    clint->dev.as.ops.write_op = clint_write;

    clint->dev.as.device = clint;

    register_address_space(parent_as, &(clint->dev.as));

    pthread_mutex_init(&clint->_mutex, NULL);
    pthread_cond_init(&clint->_cond, NULL);

    pthread_create(&tid, NULL, _routine, clint);

    return (device_t *) clint;
}
