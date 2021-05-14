pansy linux
===========
this is a linux distribution made fully from scratch - that is, we're writing
everything other than the kernel itself by hand.

we're hanging out on #homegrown at [irc.tilde.chat](https://tilde.chat/) - come say hi!

running / building
------------------
before building, you'll need to manually supply those two files:
* deps/vmlinuz - the kernel, you'll probably find one in `/boot`
* deps/e1000.ko - the ethernet driver, look for it in `/lib/modules/*/kernel/drivers/net/ethernet/intel/e1000/`

then you can just run `make boot` to get dropped into QEMU. you can press C-a x to exit the vm.
you can login as the root user - `root:password`

resources
---------
http://tilde.town/~elly/userland.txt
also, the earlier commits have the resources used in their full messages

what can i do?
--------------
    grep -r TODO
as for the bigger things, [here they are](https://github.com/dzwdz/pansy-fork/projects/2)
