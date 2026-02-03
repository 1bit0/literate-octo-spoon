/* src/boot/boot.S */
.intel_syntax noprefix

.section .multiboot
.align 4
.long 0x1BADB002          /* Magic */
.long 0x00                /* Flags */
.long -(0x1BADB002 + 0x00) /* Checksum */

.section .bss
.align 16
stack_bottom:
.skip 16384 /* 16 KiB */
stack_top:

/* Page tables */
.align 4096
p4_table:
.skip 4096
p3_table:
.skip 4096
p2_table:
.skip 4096

.section .text
.global _start
.code32
_start:
    /* Set up stack */
    mov esp, offset stack_top

    /* Set up Page Tables */
    /* Map P4[0] -> P3 */
    mov eax, offset p3_table
    or eax, 0b11 /* Present + Writable */
    mov [p4_table], eax

    /* Map P3[0] -> P2 */
    mov eax, offset p2_table
    or eax, 0b11
    mov [p3_table], eax

    /* Map P2[0..511] -> 2MB Huge Pages */
    mov ecx, 0 /* Counter */
.map_p2_table:
    /* Calculate address: ecx * 2MB */
    mov eax, 0x200000
    mul ecx
    /* Or with flags */
    or eax, 0b10000011 /* Present + Writable + Huge */

    /* Write to table */
    mov [p2_table + ecx * 8], eax
    mov dword ptr [p2_table + ecx * 8 + 4], 0

    inc ecx
    cmp ecx, 512
    jne .map_p2_table

    /* Enable PAE */
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    /* Load P4 to CR3 */
    mov eax, offset p4_table
    mov cr3, eax

    /* Enable Long Mode in EFER MSR */
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    /* Enable Paging */
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    /* Load GDT */
    lgdt [gdt64_pointer]

    /* Jump to 64-bit code */
    jmp 0x08:offset long_mode_start

.section .rodata
gdt64:
    .quad 0 /* Zero entry */
    .quad (1<<43) | (1<<44) | (1<<47) | (1<<53) /* Code segment */
gdt64_pointer:
    .word . - gdt64 - 1
    .quad gdt64

.code64
.global long_mode_start
long_mode_start:
    /* Zero out data segment registers */
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    /* Call kernel main */
    call kernel_main

    /* Halt */
    hlt
