/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 256K
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 32K
}

SECTIONS
{
    /* The startup code goes first into FLASH */
    .isr_vector :
    {
        . = ALIGN(4);
        __svector = .;
        KEEP(*(.isr_vector)) /* Startup code */
        . = ALIGN(4);
    } >FLASH
  
    /* The program code and other data goes into FLASH */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        . = ALIGN(4);
        __etext = .;
    } >FLASH
    
    .data :
    {
        . = ALIGN(4);
        __sdata = .;
        *(.data*)
        . = ALIGN(4);
        __edata = .;
    } >RAM AT>FLASH
    
    .bss :
    {
        . = ALIGN(4);
        __sbss = .;
        *(.bss*)
        . = ALIGN(4);
        __ebss = .;
    } >RAM AT>FLASH
    
	.heap (COPY):
	{
		__end__ = .;
		PROVIDE(end = .);
		*(.heap*)
		__heaplimit = .;
	} >RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY):
	{
		*(.stack*)
	} >RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__stacktop = ORIGIN(RAM) + LENGTH(RAM);
	__stacklimit = __stacktop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __stacktop);
	
	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__stacklimit >= __heaplimit, "region RAM overflowed with stack")
    
    /* Check if FLASH usage exceeds FLASH size */
    ASSERT( LENGTH(FLASH) >= (__etext + SIZEOF(.data)), "FLASH memory overflowed !")
}
