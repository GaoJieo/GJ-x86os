/**
 * 16位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 */

// 16位代码，必须加上放在开头，以便有些io指令生成为32位
__asm__(".code16gcc");

#include "loader.h"

boot_info_t boot_info;			// 启动参数信息

/**
 * BIOS下显示字符串
 */
static void show_msg (const char * msg) {
    char c;

	// 使用bios写显存，持续往下写
	while ((c = *msg++) != '\0') {  // %%会被解析为%，ch为操作符号名，r表示编译器自动选择通用寄存器存储c的值
		__asm__ __volatile__(
				"mov $0xe, %%ah\n\t"
				"mov %[ch], %%al\n\t"
				"int $0x10"::[ch]"r"(c));
	}
}

// 参考：https://wiki.osdev.org/Memory_Map_(x86)
// 1MB以下比较标准, 在1M以上会有差别
// 检测：https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15.2C_AH_.3D_0xC7
static void  detect_memory(void) {
	uint32_t contID = 0;
	SMAP_entry_t smap_entry;
	int signature, bytes;

    show_msg("try to detect memory:");

	boot_info.ram_region_count = 0;
	SMAP_entry_t * entry = &smap_entry;  // 既是输入，也是输出，所以不能是nulltptr
	for (int i = 0; i < BOOT_RAM_REGION_MAX; i++) {
		__asm__ __volatile__("int  $0x15"
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry));
		if (signature != 0x534D4150) { // 检验返回确实是SMAP
            show_msg("failed.\r\n");
			return;
		}

		// 大于20字节时需要检验ACPI是否可用
        // ACPI最低位表示：如果为 1：该内存区域可被操作系统使用；如果为 0：不能用于常规用途（例如属于 ACPI 数据结构）
		if (bytes > 20 && (entry->ACPI & 0x0001) == 0){
			continue;
		}

        // 保存RAM信息，只取32位，空间有限无需考虑更大容量的情况
        if (entry->Type == 1) {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
        }

		if (contID == 0) {
			break;
		}
	}
    show_msg("ok.\r\n");
}

// GDT表。临时用，后面内容会替换成自己的
uint16_t gdt_table[][4] = {
    {0, 0, 0, 0}, // 空描述符（NULL 段描述符）
    {0xFFFF, 0x0000, 0x9A00, 0x00CF}, // 代码段描述符，由A：1010设置
    {0xFFFF, 0x0000, 0x9200, 0x00CF},  // 数据段描述符，由2：0010设置
};

/**
 * 进入保护模式
 */
static void  enter_protect_mode() {
    // 关中断
    cli();

    // 开启A20地址线，使得可访问1M以上空间
    // 使用的是Fast A20 Gate方式，见https://wiki.osdev.org/A20#Fast_A20_Gate
    uint8_t v = inb(0x92); // 从I/O端口0x92读取当前值到变量v中
    outb(0x92, v | 0x2);   // 将v与0x2进行按位或操作后，写回到I/O端口0x92

    // 加载GDT。由于中断已经关掉，IDT不需要加载
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    // 打开CR0的保护模式位，进入保持模式
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));


    // 长跳转进入到保护模式
    // 使用长跳转，以便清空流水线，将里面的16位代码给清空，8表示使用GDT表第一个描述符
    far_jump(8, (uint32_t)protect_mode_entry);
}


void loader_entry(void) {
    show_msg("....loading.....\r\n");
	detect_memory();    
	enter_protect_mode();
    for(;;) {}
}


