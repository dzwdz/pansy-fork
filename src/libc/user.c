#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

uid_t getuid() {
	return syscall(SYS_getuid);
}
uid_t geteuid() {
	return syscall(SYS_geteuid);
}

uid_t getgid() {
	return syscall(SYS_getgid);
}
uid_t getegid() {
	return syscall(SYS_getegid);
}

int setreuid(uid_t ruid, uid_t euid) {
	return syscall(SYS_setreuid, ruid, euid);
}
int setregid(gid_t rgid, gid_t egid) {
	return syscall(SYS_setregid, rgid, egid);
}
