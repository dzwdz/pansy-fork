.PHONY: boot clean

CC     ?= gcc
CFLAGS := ${CFLAGS}
CFLAGS += -nostdlib -static
CFLAGS += -Wall -Wextra
CFLAGS += -g
CFLAGS += -Isrc/libc/

KERNEL ?= deps/vmlinuz
KFLAGS := ${KFLAGS},
KFLAGS += console=ttyS0, root=/dev/ram0, 
ifdef VERBOSE
	KFLAGS += debug=true
endif

QEMU   ?= qemu-system-x86_64
QFLAGS := ${QFLAGS}
QFLAGS += -kernel $(KERNEL) -append "${KFLAGS}" -m 1G -nographic
QFLAGS += -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::1312-:80



# files that the final image depends on
fs := $(patsubst src/bin/%.c,root/bin/%,$(wildcard src/bin/*.c))
fs += root/Users
fs += root/lib/modules/e1000.ko
fs += root/var/www/html/index.html
fs += root/var/www/html/favicon.png

initramfs.cpio.gz: $(fs)
	@cp root/bin/init root/init
	find root/ | cut -sd / -f 2- | cpio -o --format=newc -Droot/ -R root:root | gzip -9 > $@

boot: initramfs.cpio.gz
	$(QEMU) ${QFLAGS} -initrd $<

clean:
	find . -type f -name '*.o' -delete
	rm -r root/*
	rm initramfs.cpio.gz


### copy over static files ###
root/Users: $(shell find static/Users)
	@cp -r static/Users root/Users

root/lib/modules/e1000.ko: deps/e1000.ko
	@mkdir -p $(@D)
	@cp $< $@

root/%: static/%
	@mkdir -p $(@D)
	@cp -r $< $@


### build all of /bin ###
root/bin/%: src/bin/%.c root/lib/libc.a
	@mkdir -p $(@D)
	@${CC} ${CFLAGS} $^ -o $@


### build the libc ###
### every binary is statically linked against it ###
libc_obj := $(patsubst %.c,%.o,$(wildcard src/libc/*.c))
math_obj := $(patsubst %.c,%.o,$(wildcard src/libc/math/*.c))
root/lib/libc.a: src/libc/lowlevel.o $(libc_obj) $(math_obj)
	@mkdir -p $(@D)
	ar rcs $@ $^

src/libc/lowlevel.o: src/libc/lowlevel.s
	@${CC} ${CFLAGS} -c $^ -o $@

src/libc/%.o: src/libc/%.c
	@${CC} ${CFLAGS} -c $^ -o $@

src/libc/math/%.o: src/libc/math/%.c
	@${CC} ${CFLAGS} -c $^ -o $@
