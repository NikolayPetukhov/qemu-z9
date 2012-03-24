#include "sysbus.h"

#define DEBUG_MSM_IO
#include "msm.h"

typedef struct msm_a9_state {
    SysBusDevice busdev;
    MemoryRegion iomem;
    uint32_t state;
} msm_a9_state;

static const VMStateDescription msm_a9_vmsd = {
    .name = "msm_a9",
    .version_id = 1,
    .minimum_version_id = 1,
    .minimum_version_id_old = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(state, msm_a9_state),
        VMSTATE_END_OF_LIST()
    }
};

enum {
	PCOM_CMD_IDLE = 0x0,
	PCOM_CMD_DONE,
	PCOM_RESET_APPS,
	PCOM_RESET_CHIP,
	PCOM_CONFIG_NAND_MPU,
	PCOM_CONFIG_USB_CLKS,
	PCOM_GET_POWER_ON_STATUS,
	PCOM_GET_WAKE_UP_STATUS,
	PCOM_GET_BATT_LEVEL,
	PCOM_CHG_IS_CHARGING,
	PCOM_POWER_DOWN,
	PCOM_USB_PIN_CONFIG,
	PCOM_USB_PIN_SEL,
	PCOM_SET_RTC_ALARM,
	PCOM_NV_READ,
	PCOM_NV_WRITE,
	PCOM_GET_UUID_HIGH,
	PCOM_GET_UUID_LOW,
	PCOM_GET_HW_ENTROPY,
	PCOM_RPC_GPIO_TLMM_CONFIG_REMOTE,
	PCOM_CLKCTL_RPC_ENABLE,
	PCOM_CLKCTL_RPC_DISABLE,
	PCOM_CLKCTL_RPC_RESET,
	PCOM_CLKCTL_RPC_SET_FLAGS,
	PCOM_CLKCTL_RPC_SET_RATE,
	PCOM_CLKCTL_RPC_MIN_RATE,
	PCOM_CLKCTL_RPC_MAX_RATE,
	PCOM_CLKCTL_RPC_RATE,
	PCOM_CLKCTL_RPC_PLL_REQUEST,
	PCOM_CLKCTL_RPC_ENABLED,
	PCOM_VREG_SWITCH,
	PCOM_VREG_SET_LEVEL,
	PCOM_GPIO_TLMM_CONFIG_GROUP,
	PCOM_GPIO_TLMM_UNCONFIG_GROUP,
	PCOM_NV_READ_HIGH_BITS,
	PCOM_NV_WRITE_HIGH_BITS,
	PCOM_NUM_CMDS,
};

enum {
	 PCOM_INVALID_STATUS = 0x0,
	 PCOM_READY,
	 PCOM_CMD_RUNNING,
	 PCOM_CMD_SUCCESS,
	 PCOM_CMD_FAIL,
};

#define APP_COMMAND (0x00)
#define APP_STATUS  (0x04)
#define APP_DATA1   (0x08)
#define APP_DATA2   (0x0C)

#define MDM_COMMAND (0x10)
#define MDM_STATUS  (0x14)
#define MDM_DATA1   (0x18)
#define MDM_DATA2   (0x1C)

static uint64_t msm_a9_read(void *opaque,
                                        target_phys_addr_t offset,
                                        unsigned size)
{
    //msm_a9_state *s = opaque;

    //LOG_MSM_IO("%s: 0x%x\n", __FUNCTION__, offset);

    switch (offset) {
    case APP_COMMAND:
        LOG_MSM_IO("%s: APP_COMMAND = PCOM_CMD_DONE\n", __FUNCTION__);
	    return PCOM_CMD_DONE;
    case MDM_STATUS:
        LOG_MSM_IO("%s: MDM_STATUS = PCOM_READY\n", __FUNCTION__);
        return PCOM_READY;
    case APP_DATA1:
        LOG_MSM_IO("%s: APP_DATA1 = 0\n", __FUNCTION__);
        return 0;
    case APP_DATA2:
        LOG_MSM_IO("%s: APP_DATA2 = 0\n", __FUNCTION__);
        return 0;

    default:
        return 0;
    }
}

static void msm_a9_write(void *opaque, target_phys_addr_t offset,
                                     uint64_t value, unsigned size)
{
    //msm_a9_state *s = opaque;

    //LOG_MSM_IO("%s: 0x%04x\n", __FUNCTION__, offset);

    switch (offset) {
    case APP_COMMAND:
        LOG_MSM_IO("%s: APP_COMMAND = %llu\n", __FUNCTION__, value);
        break;
    case APP_DATA1:
        LOG_MSM_IO("%s: APP_DATA1 = %llu\n", __FUNCTION__, value);
        break;
    case APP_DATA2:
        LOG_MSM_IO("%s: APP_DATA2 = %llu\n", __FUNCTION__, value);
        break;
    }
}

static const MemoryRegionOps msm_a9_ops = {
    .read = msm_a9_read,
    .write = msm_a9_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static int msm_a9_init(SysBusDevice *dev)
{
    msm_a9_state *s = FROM_SYSBUS(msm_a9_state, dev);

    memory_region_init_io(&s->iomem, &msm_a9_ops, s, "msm_a9", 0x20);
    sysbus_init_mmio(dev, &s->iomem);

    return 0;
}

static SysBusDeviceInfo msm_a9_info = {
    .init = msm_a9_init,
    .qdev.name = "msm_a9",
    .qdev.size = sizeof(msm_a9_state),
    .qdev.vmsd = &msm_a9_vmsd,
};

static void msm_a9_register_devices(void)
{
    sysbus_register_withprop(&msm_a9_info);
}

device_init(msm_a9_register_devices)
// vim: set tabstop=4 shiftwidth=4 expandtab:
