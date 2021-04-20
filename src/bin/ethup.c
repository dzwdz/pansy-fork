#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define DIE(x) {puts(x); return 1;}

// see man netdevice(7)

int main() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) DIE("couldn't create socket");

    struct ifreq ifr = {0};
    strcpy((char*)&ifr.ifr_name, "eth0");

    { // todo use a byte swap function
        struct sockaddr_in *sa = (void*)&ifr.ifr_addr;
        sa->sin_family = AF_INET;
        sa->sin_addr.s_addr = 0x0f02000a; // local ip 10.0.2.15
                                          // for some reason that's the only
                                          // one that works
        if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) DIE("couldn't assign local ip");

        sa->sin_addr.s_addr = 0x00FFFFFF; // subnet mask 255.255.255.0
        if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) DIE("couldn't assign subnet mask");
    }

    // set the UP and RUNNING flags
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) DIE("couldn't get flags");
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) DIE("couldn't set flags");

    close(fd);
    puts("eth upped");
    return 0;
}
