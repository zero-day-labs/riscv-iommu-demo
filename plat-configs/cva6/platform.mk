# Architecture definition
ARCH:=riscv
# CPU definition
CPU:=
# Interrupt controller definition
IRQC:=PLIC

drivers := 8250_uart

platform_description:=cva6_desc.c
platform-cppflags =
platform-cflags = 
platform-asflags =
platform-ldflags =
