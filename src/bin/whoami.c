#include <fs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// allocates a new string
char* lookup_user(uid_t uid, gid_t gid) {
    DIR *dirp = opendir("/Users");
    
    if (!dirp) {
        closedir(dirp);
        return NULL;
    }

    struct stat statbuf;
    char* buf = malloc(128);
    strcpy(buf, "/Users/");

    while (1) {
        struct dirent *dp = readdir(dirp);
        if (!dp) break;

        strcpy(buf + 7, dp->d_name);
        if (stat(buf, &statbuf)) {
            closedir(dirp);
            return NULL;
        }
        
        if (uid != statbuf.st_uid) continue;
        if (gid != statbuf.st_gid) continue;

        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;
        
        closedir(dirp);
        return buf;
    }

    closedir(dirp);
    return NULL;
}

int main() {
    printf("UID real %x\teff %x\n", getuid(), geteuid());
    printf("GID real %x\teff %x\n", getgid(), getegid());

    printf("%s\n", lookup_user(getuid(), -1));

    return 0;
}
