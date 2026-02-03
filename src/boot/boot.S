/* src/boot/boot.S */
.intel_syntax noprefix

/* Multiboot Header */
.section .multiboot
.align 4
.long 0x1BADB002             /* Magic */
.long 0x00000007             /* Flags: Align(1) | MemInfo(2) | Video(4) */
.long -(0x1BADB002 + 0x00000007) /* Checksum */
.long 0, 0, 0, 0, 0          /* Unused address fields */
.long 0                      /* Mode Type: 0 = Linear Graphics */
.long 1024                   /* Width */
.long 768                    /* Height */
.long 32                     /* Depth */

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
/* We need 4 P2 tables to map 4GB (each maps 1GB) */
p2_table_0:
.skip 4096
p2_table_1:
.skip 4096
p2_table_2:
.skip 4096
p2_table_3:
.skip 4096

.section .text
.global _start
.code32
_start:
    /* Set up stack */
    mov esp, offset stack_top

    /* Save Multiboot Info pointer (ebx) for later */
    mov edi, ebx

    /* Set up Page Tables */

    /* Map P4[0] -> P3 */
    mov eax, offset p3_table
    or eax, 0b11 /* Present + Writable */
    mov [p4_table], eax

    /* Map P3[0] -> P2_0 */
    mov eax, offset p2_table_0
    or eax, 0b11
    mov [p3_table], eax

    /* Map P3[1] -> P2_1 */
    mov eax, offset p2_table_1
    or eax, 0b11
    mov [p3_table + 8], eax

    /* Map P3[2] -> P2_2 */
    mov eax, offset p2_table_2
    or eax, 0b11
    mov [p3_table + 16], eax

    /* Map P3[3] -> P2_3 */
    mov eax, offset p2_table_3
    or eax, 0b11
    mov [p3_table + 24], eax

    /*
       Map P2 tables to huge pages.
       We need to map 512 entries per table.
       Total 4 tables * 512 entries = 2048 entries.
       0 to 4GB.
    */

    /* Mapping P2_0 (0GB - 1GB) */
    mov ecx, 0             /* Index */
    mov ebx, 0             /* Base Address */
.map_p2_0:
    mov eax, 0x200000      /* 2MB */
    mul ecx
    or eax, 0b10000011     /* Present + Writable + Huge */
    mov [p2_table_0 + ecx * 8], eax
    mov dword ptr [p2_table_0 + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .map_p2_0

    /* Mapping P2_1 (1GB - 2GB) */
    mov ecx, 0
.map_p2_1:
    mov eax, 0x200000
    mul ecx
    add eax, 0x40000000    /* +1GB offset */
    or eax, 0b10000011
    mov [p2_table_1 + ecx * 8], eax
    mov dword ptr [p2_table_1 + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .map_p2_1

    /* Mapping P2_2 (2GB - 3GB) */
    mov ecx, 0
.map_p2_2:
    mov eax, 0x200000
    mul ecx
    add eax, 0x80000000    /* +2GB offset */
    or eax, 0b10000011
    mov [p2_table_2 + ecx * 8], eax
    mov dword ptr [p2_table_2 + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .map_p2_2

    /* Mapping P2_3 (3GB - 4GB) */
    mov ecx, 0
.map_p2_3:
    mov eax, 0x200000
    mul ecx
    add eax, 0xC0000000    /* +3GB offset */
    /* Check for carry to high dword if needed, but 3GB + 2MB*512 = 4GB fits in 32-bit register (0xFFFFFFFF) */
    /* wait, 0xC0000000 + 0x3FE00000 = 0xFFE00000. It fits. */
    or eax, 0b10000011
    mov [p2_table_3 + ecx * 8], eax
    mov dword ptr [p2_table_3 + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .map_p2_3

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

    /* Restore Multiboot Info pointer to RDI (first arg) */
    /* EDI was set to EBX in 32-bit mode */
    /* mov edi, edi clears upper 32 bits of rdi */
    mov edi, edi
    /* In 32-bit start, we moved ebx to edi. EDI is preserved during the jump? Yes, registers are preserved. */

    /* Call kernel main */
    call kernel_main

    /* Halt */
    hlt
