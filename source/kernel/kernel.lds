/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
SECTIONS
{
    . = 0x00010000;            /* 内核加载的起始地址 TODO: 起始位置会不会太小*/

	.text : {                  /* 代码段放在 0x00010000 开始 */
		*(.text)
	} 

	.rodata : {                /* 这个段的地址紧跟代码段 在链接时就会确定好*/
		*(.rodata)
	}

	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
}
