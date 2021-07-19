/*
 * RTC
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"

#define RTC_ADDRESS_SPACE_START 0x0000000000101000
#define RTC_ADDRESS_SPACE_END   0x0000000000101FFF

typedef struct _rtc_t
{
    device_t dev;
} rtc_t;


static uint64_t
rtc_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    printf("### %s: need to be implemented!\n", __func__);
    return 0;
}

static uint64_t
rtc_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    printf("### %s: need to be implemented!\n", __func__);
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
