/**
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 */

  #include "os_cfg.h"

  	// 不必加.code32因默认就是32位
 	.text
 	.global _start
	.extern kernel_init
    .extern init_main

_start:

    # 直接将传入的参数boot_info再次压入栈中，给kernel_init调用
    push 4(%esp)

    # kernel_init(boot_info)
    call kernel_init

	// 重新加载GDT
	jmp $KERNEL_SELECTOR_CS, $gdt_reload

gdt_reload:
	mov $KERNEL_SELECTOR_DS, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

	// 栈设置
	mov $(stack + KERNEL_STACK_SIZE), %esp

	// 栈和段等沿用之前的设置
	jmp init_main

	.bss
.comm stack, KERNEL_STACK_SIZE    // comm 声明未初始化的通用内存区域，以字节计


// 中断发生时，会自动切换到特权级0对应的栈中去执行
// 并且只保存ss,esp,cs,eip,flags寄存器
// 所以需要在中断中自行保存其它寄存器
		
	// 下面的代码基本上是对上面代码的重复，只不过换用了宏来处理
	// 注意确定没写错，可对照反汇编来看。另外，下面的代码没有办法调试，原因未知
   .text
.macro exception_handler name num with_error_code
	    .extern do_handler_\name
		.global exception_handler_\name
	exception_handler_\name:
		// 有错误玛时cpu自动压入
		// 如果没有错误码，手动压入一个缺省值
		// 这样堆栈就和有错误码的情形一样了
		.if \with_error_code == 0
			push $0
		.endif

		// 压入异常号
		push $\num

		// 保存所有寄存器
		pushal
		push %ds
		push %es
		push %fs
		push %gs

		// 调用中断处理函数
		push %esp
		call do_handler_\name
		add $(1*4), %esp		// 丢掉esp

		// 恢复保存的寄存器
		pop %gs
		pop %fs
		pop %es
		pop %ds
		popal

		// 跳过压入的异常号和错误码
		add $(2*4), %esp
		iret
.endm

exception_handler unknown, -1, 0
exception_handler divider, 0, 0
exception_handler Debug, 1, 0
exception_handler NMI, 2, 0
exception_handler breakpoint, 3, 0
exception_handler overflow, 4, 0
exception_handler bound_range, 5, 0
exception_handler invalid_opcode, 6, 0
exception_handler device_unavailable, 7, 0
exception_handler double_fault, 8, 1
exception_handler invalid_tss, 10, 1
exception_handler segment_not_present, 11, 1
exception_handler stack_segment_fault, 12, 1
exception_handler general_protection, 13, 1
exception_handler page_fault, 14, 1
exception_handler fpu_error, 16, 0
exception_handler alignment_check, 17, 1
exception_handler machine_check, 18, 0
exception_handler smd_exception, 19, 0
exception_handler virtual_exception, 20, 0

// 硬件中断
exception_handler timer, 0x20, 0
exception_handler kbd, 0x21, 0

// eax, ecx, edx由调用者自动保存
// ebx, esi, edi, ebp需要由被调用者保存和恢复
// cs/ds/es/fs/gs/ss不用保存，因为都是相同的, 平坦模式都为0
// esp不用保存，只需要让esp保存在调用之前的状态
// eflags ?
	.text
	.global simple_switch
simple_switch:
	movl 4(%esp), %eax   // 取from->stack
	movl 8(%esp), %edx   // 取to->stack

	// 保存前一任务的状态
	push %ebp
	push %ebx
	push %esi
	push %edi

	// 进程切换的核心，切换栈
	mov %esp, (%eax)    // from->stack = esp，main的任务栈也是在这里初始化的
  	mov %edx, %esp      // esp = to->stack

	// 加载下一任务的栈
	pop %edi
	pop %esi
	pop %ebx
	pop %ebp
  	ret
     
	.global exception_handler_syscall
    .extern do_handler_syscall
exception_handler_syscall:
	// 保存前一任务的状态
	pushal
	push %ds
	push %es
	push %fs
	push %gs
	pushfl

	// 使用内核段寄存器，避免使用应用层的
	mov $(KERNEL_SELECTOR_DS), %eax
	// 主要目的是修改这些段寄存器的CPL，修改完之后才是内核段
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs

    // 调用处理函数
    mov %esp, %eax
    push %eax
	call do_handler_syscall
	add $4, %esp

    // 再切换回来
	popf
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa
	
	// 5个参数，加上5*4，不加会导致返回时ss取不出来，最后返回出现问题
    retf $(5*4)    // CS发生了改变，需要使用远跳转