/**
 * /startup.S
 *
 * Copyright (C) 2017 HinsShum
 *
 * bc95.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

    .syntax unified  /* 指明下面的指令是arm和thumb的通用格式 */
    .cpu cortex-m3
    .fpu softvfp     /* 软浮点 */
 
    .section .stack  /* 定义栈空间，向下生长 */
    .align 3         /* 按8字节对齐 */
#ifdef __STACK_SIZE
    .equ stack_size, __STACK_SIZE
#else
    .equ stack_size, 0x0400
#endif
    .globl __stacktop
    .globl __stacklimit
__stacklimit:
    .space stack_size
    .size __stacklimit, . - __stacklimit
__stacktop:
    .size __stacktop, . - __stacktop

    .section .heap  /* 定义堆空间(如果有的话)，向上生长 */
    .align 3
#ifdef __HEAP_SIZE
    .equ heap_size, __HEAP_SIZE
#else
    .equ heap_size, 0x0000
#endif
    .globl __heapbase
    .globl __heaplimit
__heapbase:
    .if heap_size
    .space heap_size
    .endif
    .size __heapbase, . - __heapbase
__heaplimit:
    .size __heaplimit, . - __heaplimit

    .section .isr_vector,"a"            /* 定义中断向量表 */
    .align 2

__isr_vector:
    .long __stacktop                    /* Top of Stack */
    .long Reset_Handler                 /* Reset Handler */
    .long NMI_Handler                   /* NMI Handler */
    .long HardFault_Handler             /* Hard Fault Handler */
    .long MemManage_Handler             /* MPU Fault Handler */
    .long BusFault_Handler              /* Bus Fault Handler */
    .long UsageFault_Handler            /* Usage Fault Handler */
    .long 0                             /* Reserved */
    .long 0                             /* Reserved */
    .long 0                             /* Reserved */
    .long 0                             /* Reserved */
    .long SVC_Handler                   /* SVCall Handler */
    .long DebugMon_Handler              /* Debug Monitor Handler */
    .long 0                             /* Reserved */
    .long PendSV_Handler                /* PendSV Handler */
    .long SysTick_Handler               /* SysTick Handler */
    /* External Interrupts */
    .long DMA_IRQHandler                /* DMA Interrupt */
    .long GPIO_EVEN_IRQHandler          /* GPIO_EVEN Interrupt */
    .long TIMER0_IRQHandler             /* TIMER0 Interrupt */
    .long USART0_RX_IRQHandler          /* USART0_RX Interrupt */
    .long USART0_TX_IRQHandler          /* USART0_TX Interrupt */
    .long USB_IRQHandler                /* USB Interrupt */
    .long ACMP0_IRQHandler              /* ACMP0 Interrupt */
    .long ADC0_IRQHandler               /* ADC0 Interrupt */
    .long DAC0_IRQHandler               /* DAC0 Interrupt */
    .long I2C0_IRQHandler               /* I2C0 Interrupt */
    .long I2C1_IRQHandler               /* I2C1 Interrupt */
    .long GPIO_ODD_IRQHandler           /* GPIO_ODD Interrupt */
    .long TIMER1_IRQHandler             /* TIMER1 Interrupt */
    .long TIMER2_IRQHandler             /* TIMER2 Interrupt */
    .long TIMER3_IRQHandler             /* TIMER3 Interrupt */
    .long USART1_RX_IRQHandler          /* USART1_RX Interrupt */
    .long USART1_TX_IRQHandler          /* USART1_TX Interrupt */
    .long LESENSE_IRQHandler            /* LESENSE Interrupt */
    .long USART2_RX_IRQHandler          /* USART2_RX Interrupt*/
    .long USART2_TX_IRQHandler          /* USART2_TX Interrupt */
    .long UART0_RX_IRQHandler           /* UART0_RX Interrupt */
    .long UART0_TX_IRQHandler           /* UART0_TX Interrupt */
    .long UART1_RX_IRQHandler           /* UART1_RX Interrupt */
    .long UART1_TX_IRQHandler           /* UART1_TX Interrupt */
    .long LEUART0_IRQHandler            /* LEUART0 Interrupt */
    .long LEUART1_IRQHandler            /* LEUART1 Interrupt */
    .long LETIMER0_IRQHandler           /* LETIMER0 Interrupt */
    .long PCNT0_IRQHandler              /* PCNT0 Interrupt */
    .long PCNT1_IRQHandler              /* PCNT1 Interrupt */
    .long PCNT2_IRQHandler              /* PCNT2 Interrupt */
    .long RTC_IRQHandler                /* RTC Interrupt */
    .long BURTC_IRQHandler              /* BURTC Interrupt */
    .long CMU_IRQHandler                /* CMU Interrupt */
    .long VCMP_IRQHandler               /* VCMP Interrupt */
    .long LCD_IRQHandler                /* LCD Interrupt */
    .long MSC_IRQHandler                /* MSC Interrupt */
    .long AES_IRQHandler                /* AES Interrupt */
    .long EBI_IRQHandler                /* EBI Interrupt */
    .long EMU_IRQHandler                /* EMU Interrupt */
    .long 0                             /* Reserved */

    .size __isr_vector, . - __isr_vector

    .text
    .thumb
    .thumb_func
    .align 2
    .globl Reset_Handler
    .type  Reset_Handler, %function
Reset_Handler:
    ldr r0, =__etext
    ldr r1, =__sdata
    ldr r2, =__edata
copy_loop:              /*  copy 数据段 */
    cmp r1, r2
    ittt lt             /* 比较r1和r2，如果r1 < r2，则执行下面三条带有lt的指令 */
    ldrlt r3, [r0], #4
    strlt r3, [r1], #4
    blt copy_loop

    ldr r1, =__sbss
    ldr r2, =__ebss
    movs r0, #0
clean_loop:             /* 清零bss段 */
    cmp r1, r2
    itt lt
    strlt r0, [r1], #4
    blt clean_loop

#ifndef __NO_SYSTEM_INIT
    bl SystemInit       /* Call the clock system intitialization function */
#endif

#ifndef __START
#define __START _start
#endif
    bl __START

    .size Reset_Handler, . - Reset_Handler

    .align 1
    .thumb_func
    .weak Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b .
    .size Default_Handler, . - Default_Handler

    /* 中断函数指示为弱链接 */
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.set	\handler_name, Default_Handler
	.endm

    def_irq_handler NMI_Handler
    def_irq_handler HardFault_Handler
    def_irq_handler MemManage_Handler
    def_irq_handler BusFault_Handler
    def_irq_handler UsageFault_Handler
    def_irq_handler SVC_Handler
    def_irq_handler DebugMon_Handler
    def_irq_handler PendSV_Handler
    def_irq_handler SysTick_Handler
    def_irq_handler DMA_IRQHandler
    def_irq_handler GPIO_EVEN_IRQHandler
    def_irq_handler TIMER0_IRQHandler
    def_irq_handler USART0_RX_IRQHandler
    def_irq_handler USART0_TX_IRQHandler
    def_irq_handler USB_IRQHandler
    def_irq_handler ACMP0_IRQHandler
    def_irq_handler ADC0_IRQHandler
    def_irq_handler DAC0_IRQHandler
    def_irq_handler I2C0_IRQHandler
    def_irq_handler I2C1_IRQHandler
    def_irq_handler GPIO_ODD_IRQHandler
    def_irq_handler TIMER1_IRQHandler
    def_irq_handler TIMER2_IRQHandler
    def_irq_handler TIMER3_IRQHandler
    def_irq_handler USART1_RX_IRQHandler
    def_irq_handler USART1_TX_IRQHandler
    def_irq_handler LESENSE_IRQHandler
    def_irq_handler USART2_RX_IRQHandler
    def_irq_handler USART2_TX_IRQHandler
    def_irq_handler UART0_RX_IRQHandler
    def_irq_handler UART0_TX_IRQHandler
    def_irq_handler UART1_RX_IRQHandler
    def_irq_handler UART1_TX_IRQHandler
    def_irq_handler LEUART0_IRQHandler 
    def_irq_handler LEUART1_IRQHandler 
    def_irq_handler LETIMER0_IRQHandler
    def_irq_handler PCNT0_IRQHandler
    def_irq_handler PCNT1_IRQHandler
    def_irq_handler PCNT2_IRQHandler
    def_irq_handler RTC_IRQHandler
    def_irq_handler BURTC_IRQHandler
    def_irq_handler CMU_IRQHandler
    def_irq_handler VCMP_IRQHandler 
    def_irq_handler LCD_IRQHandler
    def_irq_handler MSC_IRQHandler
    def_irq_handler AES_IRQHandler
    def_irq_handler EBI_IRQHandler
    def_irq_handler EMU_IRQHandler

    .end

