#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

int nanosleep(const struct timespec *req, struct timespec *rem) {
    return syscall(SYS_nanosleep, req, rem);
}

unsigned sleep(unsigned seconds) {
    struct timespec ts = {
        .tv_sec = seconds,
        .tv_nsec = 0
    };
    if (nanosleep(&ts, &ts))
        return ts.tv_sec;
    return 0;
}
