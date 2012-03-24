#include "sysbus.h"
#include "arm-misc.h"
#include "devices.h"
#include "sysemu.h"

//#define DEBUG_MSM_IO
#include "msm.h"

/* Primary interrupt controller.  */

#define VIC_INT_SELECT0     (0x0000)  /* 1: FIQ, 0: IRQ */
#define VIC_INT_SELECT1     (0x0004)  /* 1: FIQ, 0: IRQ */
#define VIC_INT_EN0         (0x0010)
#define VIC_INT_EN1         (0x0014)
#define VIC_INT_ENCLEAR0    (0x0020)
#define VIC_INT_ENCLEAR1    (0x0024)
#define VIC_INT_ENSET0      (0x0030)
#define VIC_INT_ENSET1      (0x0034)
#define VIC_INT_TYPE0       (0x0040)  /* 1: EDGE, 0: LEVEL  */
#define VIC_INT_TYPE1       (0x0044)  /* 1: EDGE, 0: LEVEL  */
#define VIC_INT_POLARITY0   (0x0050)  /* 1: NEG, 0: POS */
#define VIC_INT_POLARITY1   (0x0054)  /* 1: NEG, 0: POS */
#define VIC_NO_PEND_VAL     (0x0060)
#define VIC_INT_MASTEREN    (0x0064)  /* 1: IRQ, 2: FIQ     */
#define VIC_PROTECTION      (0x006C)  /* 1: ENABLE          */
#define VIC_CONFIG          (0x0068)  /* 1: USE ARM1136 VIC */
#define VIC_IRQ_STATUS0     (0x0080)
#define VIC_IRQ_STATUS1     (0x0084)
#define VIC_FIQ_STATUS0     (0x0090)
#define VIC_FIQ_STATUS1     (0x0094)
#define VIC_RAW_STATUS0     (0x00A0)
#define VIC_RAW_STATUS1     (0x00A4)
#define VIC_INT_CLEAR0      (0x00B0)
#define VIC_INT_CLEAR1      (0x00B4)
#define VIC_SOFTINT0        (0x00C0)
#define VIC_SOFTINT1        (0x00C4)
#define VIC_IRQ_VEC_RD      (0x00D0)  /* pending int # */
#define VIC_IRQ_VEC_PEND_RD (0x00D4)  /* pending vector addr */
#define VIC_IRQ_VEC_WR      (0x00D8)
#define VIC_IRQ_IN_SERVICE  (0x00E0)
#define VIC_IRQ_IN_STACK    (0x00E4)
#define VIC_TEST_BUS_SEL    (0x00E8)

typedef struct msm_vic_state
{
  SysBusDevice busdev;
  uint64_t level;
  uint64_t mask;
  uint64_t irq_enabled;
  uint64_t fiq_enabled;
  qemu_irq parent_irq;
  qemu_irq parent_fiq;
} msm_vic_state;

static const VMStateDescription vmstate_msm_vic = {
    .name = "msm_vic",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT64(level, msm_vic_state),
        VMSTATE_UINT64(mask, msm_vic_state),
        VMSTATE_UINT64(irq_enabled, msm_vic_state),
        VMSTATE_UINT64(fiq_enabled, msm_vic_state),
        VMSTATE_END_OF_LIST()
    }
};

static void msm_vic_update(msm_vic_state *s)
{
    uint64_t flags;

    LOG_MSM_IO("%s: s->irq_enabled:%llu s->level:%llu\n", __FUNCTION__, s->irq_enabled, s->level);

    flags = (s->level & s->irq_enabled);
    qemu_set_irq(s->parent_irq, flags != 0);
    flags = (s->level & s->fiq_enabled);
    qemu_set_irq(s->parent_fiq, flags != 0);
}

static void msm_vic_set_irq(void *opaque, int irq, int level)
{
    msm_vic_state *s = (msm_vic_state *)opaque;

    LOG_MSM_IO("%s: irq:%u level:%u\n", __FUNCTION__, irq, level);

    if (level)
        s->level |= 1u << irq;
    else
        s->level &= ~(1u << irq);

    msm_vic_update(s);
}

static uint32_t msm_vic_read(void *opaque, target_phys_addr_t offset)
{
    msm_vic_state *s = (msm_vic_state *)opaque;

    switch (offset) {
    case VIC_IRQ_VEC_RD:
    case VIC_IRQ_VEC_PEND_RD:

        if (s->level & s->irq_enabled) {

            uint64_t index = s->level & s->irq_enabled;
            uint32_t i;
            for (i = 0; i < 64; i++) {

                if ((index >> i) & 0x1) {
                    LOG_MSM_IO("%s: VIC_IRQ_VEC_RD = %u\n", __FUNCTION__, i);
                    return i;
                }
            }

            LOG_MSM_IO("%s: VIC_IRQ_VEC_RD = %llu\n", __FUNCTION__, s->level & s->irq_enabled);
        }
        else
            return 0;
    default:
        LOG_MSM_IO("%s: 0x%x\n", __FUNCTION__, offset);
        return 0;
    }
}

static void msm_vic_write(void *opaque, target_phys_addr_t offset,
                          uint32_t value)
{
    msm_vic_state *s = (msm_vic_state *)opaque;

    switch (offset) {
    case VIC_INT_SELECT0:
        LOG_MSM_IO("%s: VIC_INT_SELECT0 = 0x%x\n", __FUNCTION__, value);
        s->fiq_enabled = value;
        break;
    case VIC_INT_SELECT1:
        LOG_MSM_IO("%s: VIC_INT_SELECT1 = 0x%x\n", __FUNCTION__, value);
        s->fiq_enabled = (uint64_t)value << 32;
        break;
    case VIC_INT_TYPE0:
        LOG_MSM_IO("%s: VIC_INT_TYPE0 = 0x%x\n", __FUNCTION__, value);
        break;
    case VIC_INT_TYPE1:
        LOG_MSM_IO("%s: VIC_INT_TYPE1 = 0x%x\n", __FUNCTION__, value);
        break;
    case VIC_INT_MASTEREN:
        LOG_MSM_IO("%s: VIC_INT_MASTEREN = 0x%x\n", __FUNCTION__, value);
        break;
    case VIC_CONFIG:
        LOG_MSM_IO("%s: VIC_CONFIG = 0x%x\n", __FUNCTION__, value);
        break;
    case VIC_INT_ENSET0:
        LOG_MSM_IO("%s: VIC_INT_ENSET0 = 0x%x\n", __FUNCTION__, value);
        s->irq_enabled |= value;
        break;
    case VIC_INT_ENSET1:
        LOG_MSM_IO("%s: VIC_INT_ENSET1 = 0x%x\n", __FUNCTION__, value);
        s->irq_enabled |= value;
        break;
    case VIC_IRQ_VEC_WR:
        LOG_MSM_IO("%s: VIC_IRQ_VEC_WR = 0x%x\n", __FUNCTION__, value);
        break;
    case VIC_INT_CLEAR0:
        LOG_MSM_IO("%s: VIC_INT_CLEAR0 = 0x%x\n", __FUNCTION__, value);
        s->level &= ~((uint64_t)value);
        //msm_vic_set_irq(s, 0, 0);
        break;
    case VIC_INT_CLEAR1:
        LOG_MSM_IO("%s: VIC_INT_CLEAR1 = 0x%x\n", __FUNCTION__, value);
        s->level &= ~(((uint64_t)value << 32) | 0xffffffff);
        //msm_vic_set_irq(s, 0, 0);
        break;

    default:
        LOG_MSM_IO("%s: 0x%x = 0x%x\n", __FUNCTION__, offset, value);
        return;
    }
    msm_vic_update(s);
}

static CPUReadMemoryFunc * const msm_vic_readfn[] = {
   msm_vic_read,
   msm_vic_read,
   msm_vic_read
};

static CPUWriteMemoryFunc * const msm_vic_writefn[] = {
   msm_vic_write,
   msm_vic_write,
   msm_vic_write
};

static int msm_vic_init(SysBusDevice *dev)
{
    msm_vic_state *s = FROM_SYSBUS(msm_vic_state, dev);
    int iomemtype;

    qdev_init_gpio_in(&dev->qdev, msm_vic_set_irq, 32);
    sysbus_init_irq(dev, &s->parent_irq);
    sysbus_init_irq(dev, &s->parent_fiq);

    iomemtype = cpu_register_io_memory(msm_vic_readfn,
                                       msm_vic_writefn, s,
                                       DEVICE_NATIVE_ENDIAN);
    sysbus_init_mmio(dev, 0x1000, iomemtype);
    
    return 0;
}

static SysBusDeviceInfo msm_vic_info = {
    .init = msm_vic_init,
    .qdev.name = "msm_vic",
    .qdev.size = sizeof(msm_vic_state),
    .qdev.vmsd = &vmstate_msm_vic,
    .qdev.no_user = 1,
};

static void ztev9_register_devices(void)
{
    sysbus_register_withprop(&msm_vic_info);
}

device_init(ztev9_register_devices)
// vim: set tabstop=4 shiftwidth=4 expandtab:
