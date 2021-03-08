#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>

int64_t* _syscall(int64_t number,
				  int64_t p1,
				  int64_t p2,
				  int64_t p3,
				  int64_t p4,
				  int64_t p5);

int main() {
	_syscall(SYS_write, 1, (int64_t) "can't C me\n", 11, 0, 0);
	return 0;
}
