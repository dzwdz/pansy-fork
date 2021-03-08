.PHONY: boot clean

QEMU = qemu-system-x86_64
kernel = /boot/vmlinuz-5.10-x86_64

boot: initramfs.cpio.gz
	 $(QEMU) -kernel $(kernel) -m 1G -nographic -append "console=ttyS0, debug=true" -initrd initramfs.cpio.gz

clean:
	find . -type f -name '*.o' -delete
	rm -r root/*
	rm initramfs.cpio.gz

initramfs.cpio.gz: root/init
	find root/ | cut -sd / -f 2- | cpio -ov --format=newc -Droot/ -R root:root | gzip -9 > initramfs.cpio.gz

root/init: src/init.c root/lib/libc.a
	@mkdir -p $(@D)
	gcc -nostdlib -static $^ -o $@


root/lib/libc.a: src/libc/lowlevel.o src/libc/syscalls.o src/libc/misc.o
	@mkdir -p $(@D)
	ar rcs $@ $^

src/libc/lowlevel.o: src/libc/lowlevel.s
	gcc -nostdlib -static -c $^ -o $@

src/libc/%.o: src/libc/%.c
	gcc -nostdlib -static -c $^ -o $@
