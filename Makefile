.PHONY: boot clean

CC     ?= gcc
CFLAGS := ${CFLAGS}
CFLAGS += -nostdlib -static
CFLAGS += -Wall -Wextra -Werror

QEMU    = qemu-system-x86_64
kernel  = /boot/vmlinuz-5.10-x86_64


initramfs.cpio.gz: root/bin/init root/bin/sh
	cp root/bin/init root/init
	find root/ | cut -sd / -f 2- | cpio -ov --format=newc -Droot/ -R root:root | gzip -9 > initramfs.cpio.gz

boot: initramfs.cpio.gz
	$(QEMU) -kernel $(kernel) -m 1G -nographic -append "console=ttyS0, debug=true, root=/dev/ram0" -initrd initramfs.cpio.gz

clean:
	find . -type f -name '*.o' -delete
	rm -r root/*
	rm initramfs.cpio.gz


root/bin/%: src/bin/%.c root/lib/libc.a
	@mkdir -p $(@D)
	${CC} ${CFLAGS} $^ -o $@


root/lib/libc.a: src/libc/lowlevel.o src/libc/syscalls.o src/libc/misc.o
	@mkdir -p $(@D)
	ar rcs $@ $^

src/libc/lowlevel.o: src/libc/lowlevel.s
	${CC} ${CFLAGS} -c $^ -o $@

src/libc/%.o: src/libc/%.c
	${CC} ${CFLAGS} -c $^ -o $@

