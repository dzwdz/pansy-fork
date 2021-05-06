#include <termios.h>
#include <sys/ioctl.h>

int tcgetattr(int fd, struct termios *t) {
    return ioctl(fd, TCGETS, t) ? -1 : 0;
}

int tcsetattr(int fd, int optionals, const struct termios *t) {
    return (optionals < 0 || optionals > 2)
        ? -1
        : ioctl(fd, TCSETS + optionals, t);
}

int tcsendbreak(int fd, int duration) {
    return ioctl(fd, duration ? TCSBRKP : TCSBRK, duration);
}

int tcdrain(int fd) {
    return ioctl(fd, TCSBRK, 1);
}

int tcflush(int fd, int queue) {
    return ioctl(fd, TCFLSH, queue);
}

int fcflow(int fd, int action) {
    return ioctl(fd, TCXONC, action);
}

pid_t tcgetsid(int fd) {
    pid_t sid;
    ioctl(fd, TIOCGSID, &sid);
    return sid < 0 ? -1 : sid;
}

void cfmakeraw(struct termios *t) {
    t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    t->c_cflag &= ~(CSIZE | PARENB);
    t->c_cflag |= CS8;
}
