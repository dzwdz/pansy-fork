.PHONY: boot clean

QEMU = qemu-system-x86_64
kernel = /boot/vmlinuz-5.10-x86_64

boot: initramfs.cpio.gz
	 $(QEMU) -kernel $(kernel) -m 1G -nographic -append "console=ttyS0, debug=true" -initrd initramfs.cpio.gz

clean:
	rm initramfs.cpio.gz
	rm -r root/*

initramfs.cpio.gz: root/init
	find root/ | cut -sd / -f 2- | cpio -ov --format=newc -Droot/ -R root:root | gzip -9 > initramfs.cpio.gz

root/init: src/init.c src/lowlevel.s
	@mkdir -p $(@D)
	gcc -nostdlib -static $^ -o $@
