.PHONY: boot clean

CC     ?= gcc
CFLAGS := ${CFLAGS}
CFLAGS += -nostdlib -static
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -g

QEMU   ?= qemu-system-x86_64

KERNEL ?= /boot/vmlinuz-5.10-x86_64
KFLAGS := ${KFLAGS},
KFLAGS += console=ttyS0, root=/dev/ram0, 
ifdef VERBOSE
	KFLAGS += debug=true
endif

initramfs.cpio.gz: root/bin/init root/bin/sh root/bin/args
	cp root/bin/init root/init
	find root/ | cut -sd / -f 2- | cpio -ov --format=newc -Droot/ -R root:root | gzip -9 > $@

boot: initramfs.cpio.gz
	$(QEMU) -kernel $(KERNEL) -m 1G -nographic -append "${KFLAGS}" -initrd $<

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

