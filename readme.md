pansy linux
===========
this is a linux distribution made fully from scratch - that is, i'm writing
everything other than the kernel itself by hand.

running / building
------------------
you need to manually supply two files - deps/vmlinuz and deps/e1000.ko
* deps/vmlinuz - the kernel, you'll probably find one in `/boot`
* deps/e1000.ko - the ethernet driver, look for it in `/lib/modules/*/kernel/drivers/net/ethernet/intel/e1000/`


resources
---------
http://tilde.town/~elly/userland.txt

https://m47r1x.github.io/posts/linux-boot/

todo list
---------
* a real malloc
* telnet/ssh/a custom protocol?
* some crypto stuff
* a text editor
* a compiler / interpreter for anything
* basic fs tools
* a \_start that's more similar to libc's - apparently checking the effective
  uid is important?
* dynamic linker
