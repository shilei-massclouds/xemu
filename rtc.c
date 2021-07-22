/*
 * RTC
 */

#include <malloc.h>
#include <util.h>

#include "address_space.h"
#include "device.h"

#define RTC_ADDRESS_SPACE_START 0x0000000000101000
#define RTC_ADDRESS_SPACE_END   0x0000000000101FFF

#define RTC_TIME_LOW            0x00
#define RTC_TIME_HIGH           0x04
#define RTC_ALARM_LOW           0x08
#define RTC_ALARM_HIGH          0x0c
#define RTC_IRQ_ENABLED         0x10
#define RTC_CLEAR_ALARM         0x14
#define RTC_ALARM_STATUS        0x18
#define RTC_CLEAR_INTERRUPT     0x1c


typedef struct _rtc_t
{
    device_t dev;

    uint32_t time_high;
    uint64_t alarm_next;
    uint32_t alarm_running;
} rtc_t;


static uint64_t
rtc_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint64_t dword = 0;
    rtc_t *rtc = (rtc_t *) dev;

    switch (addr)
    {
    case RTC_TIME_LOW:
        dword = get_clock_realtime();
        rtc->time_high = dword >> 32;
        return dword & 0xFFFFFFFF;

    case RTC_TIME_HIGH:
        return rtc->time_high;

    case RTC_ALARM_LOW:
        return rtc->alarm_next & 0xFFFFFFFF;

    case RTC_ALARM_HIGH:
        return rtc->alarm_next >> 32;

    case RTC_ALARM_STATUS:
        return rtc->alarm_running;

    default:
        panic("%s: need to be implemented! 0x%lx, %lu\n",
              __func__, addr, size);
    }

    return 0;
}

static uint64_t
rtc_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    panic("%s: need to be implemented! [0x%lx]: 0x%lx (%lu)\n",
          __func__, addr, data, size);

    return 0;
}

device_t *
rtc_init(address_space *parent_as)
{
    rtc_t *rtc;

    rtc = calloc(1, sizeof(rtc_t));
    rtc->dev.name = "rtc";

    init_address_space(&(rtc->dev.as),
                       RTC_ADDRESS_SPACE_START,
                       RTC_ADDRESS_SPACE_END);

    rtc->dev.as.ops.read_op = rtc_read;
    rtc->dev.as.ops.write_op = rtc_write;

    rtc->dev.as.device = rtc;

    register_address_space(parent_as, &(rtc->dev.as));

    return (device_t *) rtc;
}
