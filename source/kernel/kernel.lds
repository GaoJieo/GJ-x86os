/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
SECTIONS
{
	PROVIDE(kernel_base = 0x0);
    . = 0x00010000;            /* 内核加载的起始地址 TODO: 起始位置会不会太小*/
	
	PROVIDE(s_text = .);        /* 代码段起始地址*/
	.text : {                  /* 代码段放在 0x00010000 开始 */
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .text)
	} 

	.rodata : {                /* 这个段的地址紧跟代码段 在链接时就会确定好*/
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .rodata)
	}

	PROVIDE(e_text = .);       /* 代码段和只读段结束地址 text_size = e_text - s_text; */

	. = ALIGN(4096);           /* 地址向上对齐到4kb边界，使其符合分页机制要求 */
	PROVIDE(s_data = .);       /* 数据段起始地址 */

	.data : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .data)
	}
	.bss : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .bss)
	}
	
	e_data = .;

	/* 初始进程的配置：接紧着在低端1MB内存开始存储，但是运行时搬运到0x80000000处 */
	. = 0x80000000;

	PROVIDE(s_first_task = LOADADDR(.first_task));
	.first_task : AT(e_data) {
		*first_task_entry*(.text .data. rodata .data)
		*first_task*(.text .data. rodata .data)
	    *lib_syscall*(.text .data .bss)
	}
	PROVIDE(e_first_task = LOADADDR(.first_task) + SIZEOF(.first_task));

	PROVIDE(mem_free_start = e_first_task);
}
