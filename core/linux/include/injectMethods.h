#ifndef INJECT_METHODS_H
#define INJECT_METHODS_H

#include <cstdint>

// bool isElf(char *path);

// todo подумать над названием 
struct TargetInfo32
{
    // для файла 
    // uint32_t e_entry;                       // Точка входа  (для работы она нужна потом. будем тянуть по требованию)
    uint32_t e_phoff;                       // RAW смещение массива программных заголовков
    uint32_t e_shoff;                       // RAW смещение массива секционных заголовков

    uint32_t offset_payload;
    uint32_t vaddr_payload;

    // uint32_t need_sh_offset;                // RAW смещение необходимой секции
    // uint32_t need_sh_addr;                  // RVA секции в памяти (пока программа работает с so). необходимо для патчинга rela.dyn
    // uint32_t need_sh_size;                  // Размер секции rel*.dyn

    // для процесса 
    // uint32_t e_phoff;                       // RAW смещение программного заголовка
    // uint32_t offset_payload;                     // RAW/RVA/VA нагрузки
    // uint32_t orig_vaddr;                    // VA оригинальной инструкции
    // uint32_t dynamic_vaddr;                 // RVA сегмента dynamic
    // uint32_t dynamic_memsz;  
};

struct TargetInfo64
{
    // для файла 
    // uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;

    uint64_t offset_payload;
    uint64_t vaddr_payload;

    // uint64_t offset_free;

    // uint64_t need_sh_offset;
    // uint64_t need_sh_addr;    
    // uint64_t need_sh_size;    

    // // для процесса 
    // // uint64_t e_phoff;
    // uint64_t offset_payload;
    // uint64_t orig_vaddr;
    // uint64_t dynamic_vaddr;                 
    // uint64_t dynamic_memsz;   
};

struct TargetInfo
{
    uint8_t ei_class;
    bool pic;
    unsigned long int process_addr;         // Адрес загрузки. Для файлов 0

    union
    {
        struct TargetInfo32 ti32;
        struct TargetInfo64 ti64;
    };
    
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

// int saveFileTargetInfo(int fd, struct TargetInfo *ti);
int saveTargetInfo(int fd, struct TargetInfo *ti, char *target);
int injectPayload(int fd, struct TargetInfo *ti, struct Payload *payload);

int fm_e_entry(int fd, struct TargetInfo *ti, struct Payload *payload);

#endif // INJECT_METHODS_H