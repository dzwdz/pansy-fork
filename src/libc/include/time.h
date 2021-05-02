#pragma once

struct timespec {
    long tv_sec;  /* TODO: should be __kernel_old_time_t, but i can't find where it is defined */
    long tv_nsec;
};
