pansy linux
===========
this is a linux distribution made fully from scratch - that is, i'm writing
everything other than the kernel itself by hand. also, the codebase currently
mixes spaces and 4-wide tabs, so it looks absolutely awful on github. sorry

running / building
------------------
The Makefile has two hardcoded paths - the kernel and the e1000 module. Fix those and run `make boot` to boot qemu.

resources
---------
http://tilde.town/~elly/userland.txt

https://m47r1x.github.io/posts/linux-boot/

todo list
---------
* malloc
* http/telnet server
* printf
* a user system
* basic fs tools
* a \_start that's more similar to libc's - apparently checking the effective
  uid is important?
