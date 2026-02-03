# Bad Windows 11 Clone (x64 Kernel)

This is a proof-of-concept x64 operating system kernel that simulates a "Bad Windows 11" experience.
It includes a Multiboot1 bootloader, a 64-bit kernel written in C, and simulated "features" like:
- Fake boot screen.
- Endless update loops.
- Random Blue Screens of Death (BSOD).

## Building

To build the kernel, run:
```bash
make
```

This produces `myos.bin`.

## Running with QEMU

You can run the kernel directly with QEMU:

```bash
qemu-system-x86_64 -kernel myos.bin
```

Or, if you want to use GRUB (requires `grub-mkrescue` and `xorriso`):
1. Create `isodir/boot/grub/grub.cfg`:
   ```
   menuentry "Bad Windows 11" {
       multiboot /boot/myos.bin
   }
   ```
2. Copy `myos.bin` to `isodir/boot/`.
3. Run `grub-mkrescue -o myos.iso isodir`.
4. Run `qemu-system-x86_64 -cdrom myos.iso`.

## Disclaimer

This is a joke OS. It does not actually manage hardware, filesystems, or user processes beyond the initial kernel thread.
