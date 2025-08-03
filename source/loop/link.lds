ENTRY(_start)
SECTIONS
{
    /* first_task*/
	. = 0x81000000;
	.text : {
		*(*.text)
	}

	.rodata : {
		*(*.rodata)
	}

	.data : {
		*(*.data)
	}

	.bss : {
		__bss_start__ = .;
		*(*.bss)
    	__bss_end__ = . ;
	}
}
