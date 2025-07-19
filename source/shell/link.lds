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
		*(*.bss)
	}
}
