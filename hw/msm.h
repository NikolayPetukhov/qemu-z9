#ifndef QEMU_MSM_H
#define QEMU_MSM_H

#include "qemu-common.h"

//#define DEBUG_MSM_IO

#ifdef DEBUG_MSM_IO
#  define LOG_MSM_IO(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#  define LOG_MSM_IO(fmt, ...) do{ } while (0)
#endif

#define MSM_GPIO1_BASE	0xA9200000
#define MSM_GPIO2_BASE	0xA9300000

#define MSM_UART1_BASE	0xA9A00000
#define MSM_UART2_BASE	0xA9B00000
#define MSM_UART3_BASE	0xA9C00000

#define MSM_VIC_BASE	0xC0000000
#define MSM_GPT_BASE	0xC0100000
#define MSM_CSR_BASE	0xC0100000

#define MSM_MDDI_BASE 0xAA600000

#define MSM_SHARED_BASE 0x01F00000

#define INT_A9_M2A_0         0
#define INT_A9_M2A_1         1
#define INT_A9_M2A_2         2
#define INT_A9_M2A_3         3
#define INT_A9_M2A_4         4
#define INT_A9_M2A_5         5
#define INT_A9_M2A_6         6
#define INT_GP_TIMER_EXP     7
#define INT_DEBUG_TIMER_EXP  8
#define INT_UART1            9
#define INT_UART2            10
#define INT_UART3            11
#define INT_UART1_RX         12
#define INT_UART2_RX         13
#define INT_UART3_RX         14
#define INT_USB_OTG          15
#define INT_MDDI_PRI         16
#define INT_MDDI_EXT         17
#define INT_MDDI_CLIENT      18
#define INT_MDP              19
#define INT_GRAPHICS         20
#define INT_ADM_AARM         21
#define INT_ADSP_A11         22
#define INT_ADSP_A9_A11      23
#define INT_SDC1_0           24
#define INT_SDC1_1           25
#define INT_SDC2_0           26
#define INT_SDC2_1           27
#define INT_KEYSENSE         28
#define INT_TCHSCRN_SSBI     29
#define INT_TCHSCRN1         30
#define INT_TCHSCRN2         31

#define INT_GPIO_GROUP1      (32 + 0)
#define INT_GPIO_GROUP2      (32 + 1)
#define INT_PWB_I2C          (32 + 2)
#define INT_NAND_WR_ER_DONE  (32 + 3)
#define INT_NAND_OP_DONE     (32 + 4)
#define INT_SOFTRESET        (32 + 5)
#define INT_PBUS_ARM11       (32 + 6)
#define INT_AXI_MPU_SMI      (32 + 7)
#define INT_AXI_MPU_EBI1     (32 + 8)
#define INT_AD_HSSD          (32 + 9)
#define INT_ARM11_PM         (32 + 10)
#define INT_ARM11_DMA        (32 + 11)
#define INT_TSIF_IRQ         (32 + 12)
#define INT_UART1DM_IRQ      (32 + 13)
#define INT_UART1DM_RX       (32 + 14)
#define INT_USB_HS           (32 + 15)
#define INT_SDC3_0           (32 + 16)
#define INT_SDC3_1           (32 + 17)
#define INT_SDC4_0           (32 + 18)
#define INT_SDC4_1           (32 + 19)
#define INT_UART2DM_RX       (32 + 20)
#define INT_UART2DM_IRQ      (32 + 21)

#define MSM_IRQ_BIT(irq)     (1 << ((irq) & 31))

#define NR_IRQS 54

#endif /* QEMU_MSM_H */
