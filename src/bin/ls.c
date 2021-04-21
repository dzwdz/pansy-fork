#include "fs.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    char* path;
    if (argc < 2)
        path = ".";
    else
        path = argv[1];

    DIR *dirp = opendir(path);
    
    if (!dirp) {
        printf("can't open dir: %s\n", path);
        return 1;
    }

    while (1) {
        struct dirent *dp = readdir(dirp);
        if (!dp) break;

        if (dp->d_name[0] == '.') continue;

        char d_type = ((char*)dp)[dp->d_reclen-1];
        if (d_type == DT_DIR)
            printf("%s/\n", dp->d_name);
        else
            printf("%s\n", dp->d_name);
    }

    closedir(dirp);

    return 0;
}
