#ifndef INJECT_H
#define INJECT_H

// Методы перехода на полезную нагрузку для файла
enum MethodsJumpsFile {
    FM_E_ENTRY, 
    FM_INIT, 
    FM_INIT_ARRAY, 
    FM_FINI, 
    FM_FINI_ARRAY, 
    FM_PLT
};

// Методы перехода на полезную нагрузку для процесса
enum MethodsJumpsProc {
    PM_IP, 
    PM_CUR_INST, 
    PM_GOT, 
    PM_FINI, 
    PM_FINI_ARRAY
};

enum TypeTarget {
    TT_FILE,
    TT_PROC
};

enum InjectStatus {
    IJS_SUCCESS,
    IJS_PARTIAL_SUCCESS,
    IJS_INJECT_ERROR,
    IJS_TARGET_NOT_FOUND,
    IJS_ACCESS_DENIED,
    // IJS_TARGET_NOT_ELF,
    IJS_OPEN_ERROR,
    IJS_PARSE_ERROR,
    IJS_UNKNOWN_TARGET_TYPE,
    IJS_UNKNOWN_JUMP_METHOD
};

struct Payload {
    int size;
    void *addr;
};

enum InjectStatus injectPayloadManager(enum TypeTarget typeTarget, char *target, struct Payload *payload, int jmpMethod);

#endif // INJECT_H