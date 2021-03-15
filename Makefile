.PHONY: boot clean

CC     ?= gcc
CFLAGS := ${CFLAGS}
CFLAGS += -nostdlib -static
CFLAGS += -Wall -Wextra
CFLAGS += -g
CFLAGS += -Isrc/libc/

KERNEL ?= /boot/vmlinuz-5.10-x86_64
KFLAGS := ${KFLAGS},
KFLAGS += console=ttyS0, root=/dev/ram0, 
ifdef VERBOSE
	KFLAGS += debug=true
endif

QEMU   ?= qemu-system-x86_64
QFLAGS := ${QFLAGS}
QFLAGS += -kernel $(KERNEL) -append "${KFLAGS}" -m 1G -nographic
QFLAGS += -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::1312-:1312



# files that the final image depends on
fs := $(patsubst src/bin/%.c,root/bin/%,$(wildcard src/bin/*.c))
fs += root/Users
fs += root/lib/modules/e1000.ko

initramfs.cpio.gz: $(fs)
	cp root/bin/init root/init
	find root/ | cut -sd / -f 2- | cpio -ov --format=newc -Droot/ -R root:root | gzip -9 > $@

boot: initramfs.cpio.gz
	$(QEMU) ${QFLAGS} -initrd $<

clean:
	find . -type f -name '*.o' -delete
	rm -r root/*
	rm initramfs.cpio.gz


root/Users: $(shell find static/Users)
	cp -r static/Users root/Users

root/lib/modules/e1000.ko: /lib/modules/5.10.18-1-MANJARO/kernel/drivers/net/ethernet/intel/e1000/e1000.ko.xz
	@mkdir -p $(@D)
	@xz -cd $< > $@

root/bin/%: src/bin/%.c root/lib/libc.a
	@mkdir -p $(@D)
	${CC} ${CFLAGS} $^ -o $@


libc_obj := $(patsubst %.c,%.o,$(wildcard src/libc/*.c))
root/lib/libc.a: src/libc/lowlevel.o $(libc_obj)
	@mkdir -p $(@D)
	ar rcs $@ $^

src/libc/lowlevel.o: src/libc/lowlevel.s
	${CC} ${CFLAGS} -c $^ -o $@

src/libc/%.o: src/libc/%.c
	${CC} ${CFLAGS} -c $^ -o $@

