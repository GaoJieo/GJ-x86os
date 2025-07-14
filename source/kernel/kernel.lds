/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
SECTIONS
{
	PROVIDE(kernel_base = 0x0);
    . = 0x00010000;            /* 内核加载的起始地址 TODO: 起始位置会不会太小*/
	
	PROVIDE(s_text = .);        /* 代码段起始地址*/
	.text : {                  /* 代码段放在 0x00010000 开始 */
		*(.text)
	} 

	.rodata : {                /* 这个段的地址紧跟代码段 在链接时就会确定好*/
		*(.rodata)
	}

	PROVIDE(e_text = .);       /* 代码段和只读段结束地址 text_size = e_text - s_text; */

	. = ALIGN(4096);           /* 地址向上对齐到4kb边界，使其符合分页机制要求 */
	PROVIDE(s_data = .);       /* 数据段起始地址 */

	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}

	PROVIDE(mem_free_start = .);   /* 内核结束的地址，也是内存空闲区的起始地址*/
}
