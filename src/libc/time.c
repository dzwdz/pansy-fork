#include <time.h>

unsigned sleep(unsigned seconds) {
	struct timespec ts = {
		.tv_sec = seconds,
		.tv_nsec = 0
	};
	if (nanosleep(&ts, &ts))
		return ts.tv_sec;
	return 0;
}
