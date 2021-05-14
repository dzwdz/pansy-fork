export # make sure sub-makes get all our vars
.PHONY: boot clean nested

CC     ?= gcc
CFLAGS := ${CFLAGS}
CFLAGS += -nostdlib -static
CFLAGS += -Wall -Wextra
CFLAGS += -g
CFLAGS += --sysroot=src
CFLAGS += -Isrc/libc/include -O2

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
fs += $(patsubst static/%,root/%,$(shell find static/))
fs += root/lib/modules/e1000.ko

initramfs.cpio.gz: $(fs) nested
	@cp root/bin/init root/init
	find root/ | cut -sd / -f 2- | cpio -o --format=newc -Droot/ -R root:root | gzip -9 > $@

boot: initramfs.cpio.gz
	$(QEMU) ${QFLAGS} -initrd $<

clean:
	find . -type f -name '*.o' -delete
	rm -r root/*
	rm initramfs.cpio.gz


### copy over static files ###
root/lib/modules/e1000.ko: deps/e1000.ko
	@mkdir -p $(@D)
	@cp $< $@

root/%: static/%
	@mkdir -p $(@D)
	@cp -r $< $@


### build all of /bin ###
# the single file build is simple enough
root/bin/%: src/bin/%.c root/lib/libc.a
	@mkdir -p $(@D)
	@${CC} ${CFLAGS} $^ -o $@

# but the thing is, some binaries have multiple source files
# we build those using this mess
M := $(shell find . -type d | grep src/bin/)
nested: root/lib/libc.a
	@for dir in $(M); do \
		DIR=$$dir $(MAKE) -f Cursedfile --no-print-directory; \
		if ! [ 0 -eq $$? ]; then \
			exit 1; \
		fi; \
	done



### build the libc ###
### every binary is statically linked against it ###
libc_obj := $(patsubst %.c,%.o,$(shell find src/libc/ -type f -name '*.c'))
root/lib/libc.a: src/libc/lowlevel.o $(libc_obj)
	@mkdir -p $(@D)
	ar rcs $@ $^

src/libc/lowlevel.o: src/libc/lowlevel.s
	@${CC} ${CFLAGS} -c $^ -o $@

src/libc/%.o: src/libc/%.c
	@${CC} ${CFLAGS} -c $^ -o $@


### copy over documentation
root/docs/%: docs/%
	@mkdir -p $(@D)
	@cp $< $@
