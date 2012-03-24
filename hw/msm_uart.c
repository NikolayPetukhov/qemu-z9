#include "sysbus.h"
#include "qemu-char.h"

//#define DEBUG_MSM_IO
#include "msm.h"

typedef struct {
    SysBusDevice busdev;
    CharDriverState *chr;
    qemu_irq irq;
} msm_uart_state;

#define UART_MR1         0x0000

#define UART_MR1_AUTO_RFR_LEVEL0(n) (((n) & 0x3f) << 8)
#define UART_MR1_RX_RDY_CTL    (1 << 7)
#define UART_MR1_CTS_CTL       (1 << 6)
#define UART_MR1_AUTO_RFR_LEVEL1(n) ((n) & 0x3f)

#define UART_MR2         0x0004
#define UART_MR2_ERROR_MODE        (1 << 6)
#define UART_MR2_BITS_PER_CHAR_5   (0 << 4)
#define UART_MR2_BITS_PER_CHAR_6   (1 << 4)
#define UART_MR2_BITS_PER_CHAR_7   (2 << 4)
#define UART_MR2_BITS_PER_CHAR_8   (3 << 4)
#define UART_MR2_STOP_BIT_LEN_0563 (0 << 2)
#define UART_MR2_STOP_BIT_LEN_1000 (1 << 2)
#define UART_MR2_STOP_BIT_LEN_1563 (2 << 2)
#define UART_MR2_STOP_BIT_LEN_2000 (3 << 2)
#define UART_MR2_PARITY_MODE_NONE  (0)
#define UART_MR2_PARITY_MODE_ODD   (1)
#define UART_MR2_PARITY_MODE_EVEN  (2)
#define UART_MR2_PARITY_MODE_SPACE (3)

#define UART_CSR         0x0008
#define UART_CSR_115200  0xFF
#define UART_CSR_57600   0xEE
#define UART_CSR_38400   0xDD
#define UART_CSR_19200   0xBB

#define UART_TF          0x000C

#define UART_CR          0x0010
#define UART_CR_CMD_NULL           (0 << 4)
#define UART_CR_CMD_RESET_RX       (1 << 4)
#define UART_CR_CMD_RESET_TX       (2 << 4)
#define UART_CR_CMD_RESET_ERR      (3 << 4)
#define UART_CR_CMD_RESET_BCI      (4 << 4)
#define UART_CR_CMD_START_BREAK    (5 << 4)
#define UART_CR_CMD_STOP_BREAK     (6 << 4)
#define UART_CR_CMD_RESET_CTS_N    (7 << 4)
#define UART_CR_CMD_PACKET_MODE    (9 << 4)
#define UART_CR_CMD_MODE_RESET     (12<< 4)
#define UART_CR_CMD_SET_RFR_N      (13<< 4)
#define UART_CR_CMD_RESET_RFR_ND   (14<< 4)
#define UART_CR_TX_DISABLE         (1 << 3)
#define UART_CR_TX_ENABLE          (1 << 3)
#define UART_CR_RX_DISABLE         (1 << 3)
#define UART_CR_RX_ENABLE          (1 << 3)

#define UART_IMR         0x0014
#define UART_IMR_RXLEV (1 << 4)
#define UART_IMR_TXLEV (1 << 0)

#define UART_IPR         0x0018
#define UART_TFWR        0x001C
#define UART_RFWR        0x0020
#define UART_HCR         0x0024

#define UART_MREG        0x0028
#define UART_NREG        0x002C
#define UART_DREG        0x0030
#define UART_MNDREG      0x0034
#define UART_IRDA        0x0038
#define UART_MISR_MODE   0x0040
#define UART_MISR_RESET  0x0044
#define UART_MISR_EXPORT 0x0048
#define UART_MISR_VAL    0x004C
#define UART_TEST_CTRL   0x0050

#define UART_SR          0x0008
#define UART_SR_HUNT_CHAR      (1 << 7)
#define UART_SR_RX_BREAK       (1 << 6)
#define UART_SR_PAR_FRAME_ERR  (1 << 5)
#define UART_SR_OVERRUN        (1 << 4)
#define UART_SR_TX_EMPTY       (1 << 3)
#define UART_SR_TX_READY       (1 << 2)
#define UART_SR_RX_FULL        (1 << 1)
#define UART_SR_RX_READY       (1 << 0)

#define UART_RF          0x000C
#define UART_MISR        0x0010
#define UART_ISR         0x0014

static void msm_uart_update(msm_uart_state *s)
{
}

static uint32_t msm_uart_read(void *opaque, target_phys_addr_t offset)
{
    msm_uart_state *s = (msm_uart_state *)opaque;
    uint32_t c;

    switch (offset) {

    case UART_SR:
        //LOG_MSM_IO("%s: UART_SR = %u\n", __FUNCTION__, UART_SR_TX_READY);
        return UART_SR_TX_READY;

    default:
        LOG_MSM_IO("%s: 0x%x\n", __FUNCTION__, offset);
        return 0;
    }
}

static void msm_uart_set_read_trigger(msm_uart_state *s)
{
}

static void msm_uart_write(void *opaque, target_phys_addr_t offset,
                          uint32_t value)
{
    msm_uart_state *s = (msm_uart_state *)opaque;
    unsigned char ch;

    switch (offset) {

    case UART_TF:
        ch = value;
        if (s->chr)
            qemu_chr_fe_write(s->chr, &ch, 1);
        break;
    default:
        LOG_MSM_IO("%s: 0x%x = 0x%x\n", __FUNCTION__, offset, value);
        break;
    }
}

static int msm_uart_can_receive(void *opaque)
{
    msm_uart_state *s = (msm_uart_state *)opaque;

    return 0;
}

static void msm_uart_receive(void *opaque, const uint8_t *buf, int size)
{
}

static void msm_uart_event(void *opaque, int event)
{
}

static CPUReadMemoryFunc * const msm_uart_readfn[] = {
   msm_uart_read,
   msm_uart_read,
   msm_uart_read
};

static CPUWriteMemoryFunc * const msm_uart_writefn[] = {
   msm_uart_write,
   msm_uart_write,
   msm_uart_write
};

static const VMStateDescription vmstate_msm_uart = {
    .name = "msm_uart",
    .version_id = 1,
    .minimum_version_id = 1,
    .minimum_version_id_old = 1,
    .fields      = (VMStateField[]) {
        VMSTATE_END_OF_LIST()
    }
};

static int msm_uart_init(SysBusDevice *dev)
{
    int iomemtype;
    msm_uart_state *s = FROM_SYSBUS(msm_uart_state, dev);

    iomemtype = cpu_register_io_memory(msm_uart_readfn,
                                       msm_uart_writefn, s,
                                       DEVICE_NATIVE_ENDIAN);

    sysbus_init_mmio(dev, 0x100, iomemtype);
    sysbus_init_irq(dev, &s->irq);
    s->chr = qdev_init_chardev(&dev->qdev);

    if (s->chr) {
        qemu_chr_add_handlers(s->chr, msm_uart_can_receive, msm_uart_receive,
                              msm_uart_event, s);
    }
    vmstate_register(&dev->qdev, -1, &vmstate_msm_uart, s);
    return 0;
}

static void msm_uart_register_devices(void)
{
    sysbus_register_dev("msm_uart", sizeof(msm_uart_state), msm_uart_init);
}

device_init(msm_uart_register_devices)
// vim: set tabstop=4 shiftwidth=4 expandtab:
