/**
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 */	
	#include "boot.h"

  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start
	.extern boot_entry
_start:
	// 重置数据段寄存器
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	// 根据https://wiki.osdev.org/Memory_Map_(x86)
	// 使用0x7c00之前的空间作栈，大约有30KB的RAM，足够boot和loader使用
	mov $_start, %esp

	// 显示boot加载完成提示
	mov $0xe, %ah        // AH=0xE: display char模式，显示一个字符
	mov $'L', %al        // AL='L': 要显示的字符
	int $0x10            // 调用 BIOS 的 int 0x10 中断服务，显示字符 'L'

	// 加载loader，只支持磁盘1
	// https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
read_loader:
	mov $0x8000, %bx	// BX=0x8000: 将 loader 读入内存的起始地址
	mov $0x2, %cx		// CH=磁道号 0x00，CL=起始扇区号 0x02（boot在0x01, 此处读的是loader的磁盘位置)
	mov $0x2, %ah		// AH=0x02: BIOS 磁盘读命令
	mov $64, %al		// AL=64: 要读取的扇区数量（每个扇区 512 字节 → 共 32KB）
	mov $0x0080, %dx	// dh: 磁头号，dl驱动器号0x80(磁盘1)
	int $0x13           // 调用 BIOS 的 int 0x13 中断进行磁盘读取
	jc read_loader      // 如果 CF=1（出错），则跳回 read_loader 重试

	// 跳转至c部分执行，再由c部分做一些处理
	jmp boot_entry

	// 原地跳转
	jmp .

	// 引导结束段
	.section boot_end, "ax"
boot_sig: .byte 0x55, 0xaa   // 第一个扇区末尾，引导程序标志