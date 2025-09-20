#include "core/injectMethods.h"

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>

bool isElf(char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return false;
    }

    unsigned char magic[4];
    ssize_t bytesRead = read(fd, magic, 4);
    close(fd);

    if (bytesRead != 4) {
        return false;
    }

    return (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}