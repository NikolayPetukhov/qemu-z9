#include "sysbus.h"
#include "qemu-timer.h"

//#define DEBUG_MSM_IO
#include "msm.h"

#define GPT_MATCH_VAL        0x0000
#define GPT_COUNT_VAL        0x0004
#define GPT_ENABLE           0x0008
#define GPT_ENABLE_CLR_ON_MATCH_EN        2
#define GPT_ENABLE_EN                     1
#define GPT_CLEAR            0x000C

#define DGT_MATCH_VAL        0x0010
#define DGT_COUNT_VAL        0x0014
#define DGT_ENABLE           0x0018
#define DGT_ENABLE_CLR_ON_MATCH_EN        2
#define DGT_ENABLE_EN                     1
#define DGT_CLEAR            0x001C

#define SPSS_TIMER_STATUS    0x0034

typedef struct {
    SysBusDevice busdev;
    MemoryRegion iomem;
    QEMUTimer *timer;
    qemu_irq irq;

    uint32_t enable;
    uint32_t match_val;
    uint32_t count_val;
} msm_gpt_state;

static const VMStateDescription vmstate_msm_gpt = {
    .name = "msm_gpt",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(enable, msm_gpt_state),
        VMSTATE_UINT32(match_val, msm_gpt_state),
        VMSTATE_UINT32(count_val, msm_gpt_state),
        VMSTATE_END_OF_LIST()
    }
};

static void msm_gpt_update(msm_gpt_state *s)
{
    qemu_set_irq(s->irq, s->enable);
}

static void msm_gpt_set_alarm(msm_gpt_state *s);

static void msm_gpt_interrupt(void * opaque)
{
    msm_gpt_state *s = (msm_gpt_state *)opaque;

    s->count_val = 0;

    LOG_MSM_IO("Alarm raised\n");
    msm_gpt_update(s);
    msm_gpt_set_alarm(s);
}

#define DGT_CLOCK 19200000

static uint32_t msm_gpt_get_count(msm_gpt_state *s)
{
    /* This assumes qemu_get_clock_ns returns the time since the machine was
       created.  */
    return s->count_val;
}

static void msm_gpt_set_alarm(msm_gpt_state *s)
{
    int64_t now;
    int64_t t;
    int64_t wait;
    uint32_t ticks;

    now = qemu_get_clock_ns(vm_clock);
    ticks = s->match_val - s->count_val;

    LOG_MSM_IO("Alarm set in %u ticks, t %llu, wait %llu\n", ticks, t, wait);

    if (ticks == 0) {
        qemu_del_timer(s->timer);
        msm_gpt_interrupt(s);
    } else {
        t = DGT_CLOCK/ticks;
        wait = get_ticks_per_sec()/t;
        qemu_mod_timer(s->timer, now + wait);
    }
}

static uint64_t msm_gpt_read(void *opaque, target_phys_addr_t offset, unsigned size)
{
    msm_gpt_state *s = (msm_gpt_state *)opaque;

    switch (offset) {
    case DGT_COUNT_VAL:
        LOG_MSM_IO("%s: COUNTVAL(0x%x)\n", __FUNCTION__, 0);
        return msm_gpt_get_count(s);

    default:
        LOG_MSM_IO("%s: 0x%x\n", __FUNCTION__, offset);
        break;
    }

    return 0;
}

static void msm_gpt_write(void *opaque, target_phys_addr_t offset,
                        uint64_t value, unsigned size)
{
    msm_gpt_state *s = (msm_gpt_state *)opaque;

    switch (offset) {
    case DGT_ENABLE:
        LOG_MSM_IO("%s: DGT_ENABLE = 0x%x\n", __FUNCTION__, value);
        if (!value) {

            LOG_MSM_IO("Timer stop\n");
            s->enable = 0;
        }
        else if (value == 0x3) {

            LOG_MSM_IO("Timer start\n");
            s->enable = 1;
            msm_gpt_set_alarm(s);
        }
        break;

    case DGT_MATCH_VAL:
        LOG_MSM_IO("%s: DGT_MATH_VAL = 0x%x\n", __FUNCTION__, value);
        s->match_val = value;
        break;
    case DGT_CLEAR:
        LOG_MSM_IO("%s: DGT_CLEAR = 0x%x\n", __FUNCTION__, value);
        s->count_val = 0;
        break;
    default:
        LOG_MSM_IO("%s: 0x%x = 0x%x\n", __FUNCTION__, offset, value);
        break;
    }
}

static const MemoryRegionOps msm_gpt_ops = {
    .read = msm_gpt_read,
    .write = msm_gpt_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static int msm_gpt_init(SysBusDevice *dev)
{
    msm_gpt_state *s = FROM_SYSBUS(msm_gpt_state, dev);

    memory_region_init_io(&s->iomem, &msm_gpt_ops, s, "msm_gpt", 0x40);
    sysbus_init_mmio(dev, &s->iomem);

    sysbus_init_irq(dev, &s->irq);
    LOG_MSM_IO("%s: irq %u\n", __FUNCTION__, s->irq);

    s->timer = qemu_new_timer_ns(vm_clock, msm_gpt_interrupt, s);

    return 0;
}

static SysBusDeviceInfo msm_gpt_info = {
    .init = msm_gpt_init,
    .qdev.name = "msm_gpt",
    .qdev.size = sizeof(msm_gpt_state),
    .qdev.vmsd = &vmstate_msm_gpt,
    .qdev.no_user = 1,
};

static void msm_gpt_register_devices(void)
{
    sysbus_register_withprop(&msm_gpt_info);
}

device_init(msm_gpt_register_devices)
// vim: set tabstop=4 shiftwidth=4 expandtab:
