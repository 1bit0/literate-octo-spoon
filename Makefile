CC = gcc
AS = as
LD = ld

CFLAGS = -ffreestanding -mno-red-zone -m64 -fno-builtin -fno-stack-protector -Wall -Wextra -O2
ASFLAGS =
LDFLAGS = -n -T src/linker.ld -z max-page-size=0x1000

KERNEL_OBJS = src/boot/boot.o src/kernel/main.o

all: myos.bin

myos.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

src/boot/boot.o: src/boot/boot.S
	$(AS) $(ASFLAGS) -o $@ $<

src/kernel/main.o: src/kernel/main.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f myos.bin $(KERNEL_OBJS)

.PHONY: all clean
