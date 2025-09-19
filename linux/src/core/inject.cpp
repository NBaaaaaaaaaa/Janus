#include "core/inject.h"
#include "core/injectMethods.h"

static enum InjectStatus injectPayloadFile(char *target, struct Payload *payload, int jmpMethod);
static enum InjectStatus injectPayloadProc(char *target, struct Payload *payload, int jmpMethod);


int injectPayloadManager(enum TypeTarget typeTarget, char *target, struct Payload *payload, int jmpMethod) {
    enum InjectStatus ijsStatus;

    if (typeTarget == TT_FILE) {
        ijsStatus = injectPayloadFile(target, payload, jmpMethod);
    } else if (typeTarget == TT_PROC) {
        ijsStatus = injectPayloadProc(target, payload, jmpMethod);
    } else {
        return IJS_UNKNOWN_TARGET_TYPE;
    }

    return ijsStatus;
}

static enum InjectStatus injectPayloadFile(char *target, struct Payload *payload, int jmpMethod) {
    enum InjectStatus ijsStatus;

    // тут функция парсинга структуры файла 

    switch (jmpMethod)
    {
    case FM_E_ENTRY:
        /* code */
        break;

    case FM_INIT:
        /* code */
        break;
        
    case FM_INIT_ARRAY:
        /* code */
        break;
        
    case FM_FINI:
        /* code */
        break;
        
    case FM_FINI_ARRAY:
        /* code */
        break;
        
    case FM_PLT:
        /* code */
        break;
    
    default:
        ijsStatus = IJS_UNKNOWN_JUMP_METHOD;
        break;
    }

    return ijsStatus;
}

static enum InjectStatus injectPayloadProc(char *target, struct Payload *payload, int jmpMethod) {
    enum InjectStatus ijsStatus;

    // тут функция парсинга структуры файла 

    switch (jmpMethod)
    {
    case PM_IP:
        /* code */
        break;

    case PM_CUR_INST:
        /* code */
        break;
        
    case PM_GOT:
        /* code */
        break;
        
    case PM_FINI:
        /* code */
        break;
        
    case PM_FINI_ARRAY:
        /* code */
        break;
    
    default:
        ijsStatus = IJS_UNKNOWN_JUMP_METHOD;
        break;
    }

    return ijsStatus;
}



