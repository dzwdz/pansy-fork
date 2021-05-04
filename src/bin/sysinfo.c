/* system status command (really just a wrapper for sysinfo(2)) */

#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    struct sysinfo info;
    if (sysinfo(&info) == -1) {
        dprintf(STDERR_FILENO, "t: error calling sysinfo\n");
        return 1;
    }

    /* assume mem_unit is 1, which it is */
    printf("Uptime (seconds):         %ld\n"
           "Load averages:            %ld  %ld  %ld\n"
           "Total memory (KiB):       %ld\n"
           "Free memory (KiB):        %ld\n"
           "Memory in use (KiB):      %ld\n"
           "Shared memory (KiB):      %ld\n"
           "Buffer memory (KiB):      %ld\n"
           "Total swap (KiB):         %ld\n"
           "Free swap (KiB):          %ld\n"
           "Swap in use (KiB):        %ld\n"
           "Total high memory (KiB):  %ld\n"
           "Free high memory (KiB):   %ld\n"
           "High memory in use (KiB): %ld\n"
           "Processes running:        %d\n",
           info.uptime,
           info.loads[0], info.loads[1], info.loads[2],
           info.totalram / 1024,
           info.freeram / 1024,
           (info.totalram - info.freeram) / 1024,
           info.sharedram / 1024,
           info.bufferram / 1024,
           info.totalswap / 1024,
           info.freeswap / 1024,
           (info.totalswap - info.freeswap) / 1024,
           info.totalhigh / 1024,
           info.freehigh / 1024,
           (info.totalhigh - info.freehigh) / 1024,
           info.procs
        );
}
