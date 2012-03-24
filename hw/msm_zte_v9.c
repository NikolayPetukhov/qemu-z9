#include "sysbus.h"
#include "arm-misc.h"
#include "primecell.h"
#include "devices.h"
#include "net.h"
#include "sysemu.h"
#include "usb-ohci.h"
#include "boards.h"
#include "loader.h"
#include "msm.h"

static struct arm_boot_info ztev9_binfo = {
    .loader_start = 0x25f8000, /* + KERNEL_LOAD_ADDR in arm_bootloader = 0x2608000*/
    .ram_size = 0x04000000,
    .board_id = 0xf656f,
};

/* Board init.  */

static void ztev9_init(ram_addr_t ram_size,
                     const char *boot_device,
                     const char *kernel_filename, const char *kernel_cmdline,
                     const char *initrd_filename, const char *cpu_model)
{
    CPUState *env;
    ram_addr_t ram_offset;
    qemu_irq *cpu_pic;
    qemu_irq pic[32];
    DeviceState *dev, *sysctl;
    int n;
    int rom_size, rom_loaded = 0;

    if (!cpu_model)
        cpu_model = "arm1136";
    env = cpu_init(cpu_model);
    if (!env) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }
    ram_offset = qemu_ram_alloc(NULL, "ztev9.ram", ram_size);
    /* ??? RAM should repeat to fill physical memory space.  */
    /* SDRAM at address zero.  */
    cpu_register_physical_memory(0, ram_size, ram_offset | IO_MEM_RAM);

    sysctl = qdev_create(NULL, "realview_sysctl");
    qdev_prop_set_uint32(sysctl, "sys_id", 0x41007004);
    qdev_init_nofail(sysctl);
    qdev_prop_set_uint32(sysctl, "proc_id", 0x02000000);
    sysbus_mmio_map(sysbus_from_qdev(sysctl), 0, 0x10000000);

    cpu_pic = arm_pic_init_cpu(env);
    dev = sysbus_create_varargs("msm_vic", MSM_VIC_BASE,
                                cpu_pic[ARM_PIC_CPU_IRQ],
                                cpu_pic[ARM_PIC_CPU_FIQ], NULL);
    for (n = 0; n < 32; n++) {
        pic[n] = qdev_get_gpio_in(dev, n);
    }

    sysbus_create_simple("msm_gpt", MSM_GPT_BASE, pic[INT_DEBUG_TIMER_EXP]);
    sysbus_create_simple("msm_a9", MSM_SHARED_BASE, NULL);

    sysbus_create_simple("msm_uart", MSM_UART3_BASE, pic[14]);
    sysbus_create_simple("msm_uart", MSM_UART1_BASE, pic[12]);
    sysbus_create_simple("msm_uart", MSM_UART2_BASE, pic[13]);

    sysbus_create_simple("msm_mddi", MSM_MDDI_BASE, NULL);

    /* Setup initial (reset) machine state */
    if (nb_option_roms) {
        rom_size = get_image_size(option_rom[0].name);
        if (rom_size > 0) {
            rom_size = load_image_targphys(option_rom[0].name, 0x0, ram_size - rom_size);
            rom_loaded = 1;
        }
        if (rom_size < 0) {
            fprintf(stderr, "%s: error loading '%s'\n",
                            __FUNCTION__, option_rom[0].name);
        }
    }

    if (!rom_loaded && !kernel_filename) {
        fprintf(stderr, "Kernel or ROM image must be specified\n");
        exit(1);
    }

    /* Load the kernel.  */
    if (kernel_filename) {
        ztev9_binfo.kernel_filename = kernel_filename;
        ztev9_binfo.kernel_cmdline = kernel_cmdline;
        ztev9_binfo.initrd_filename = initrd_filename;
        arm_load_kernel(env, &ztev9_binfo);
    }
}

static QEMUMachine ztev9_machine = {
    .name = "ztev9",
    .desc = "ZTE V9 (ARM1136EJ-S)",
    .init = ztev9_init,
    .use_scsi = 1,
};

static void ztev9_machine_init(void)
{
    qemu_register_machine(&ztev9_machine);
}

machine_init(ztev9_machine_init);
// vim: set tabstop=4 shiftwidth=4 expandtab:
