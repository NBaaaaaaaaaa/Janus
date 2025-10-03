#include "core_api.h"
#include "injectMethods.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <QDebug>

static enum InjectStatus injectPayloadFile(char *target, struct Payload *payload, int jmpMethod);
static enum InjectStatus injectPayloadProc(char *target, struct Payload *payload, int jmpMethod);


enum InjectStatus injectPayloadManager(enum TypeTarget typeTarget, char *target, struct Payload *payload, int jmpMethod) {
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
    struct TargetInfo ti;

    int fd = open(target, O_RDWR);

    if (fd == -1) {
        switch (errno) {
        case ENOENT:
            return IJS_TARGET_NOT_FOUND;
        case EACCES:
        case EPERM:
            return IJS_ACCESS_DENIED;
        default:
            return IJS_OPEN_ERROR;
        }
    }

    if (saveTargetInfo(fd, &ti, NULL)) {
        return IJS_PARSE_ERROR;
    }

    if (injectPayload(fd, &ti, payload)) {
        return IJS_INJECT_ERROR;
    }

    switch (jmpMethod)
    {
    case FM_E_ENTRY:
        fm_e_entry(fd, &ti, payload);
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

    close(fd);
    return ijsStatus;
}

static enum InjectStatus injectPayloadProc(char *target, struct Payload *payload, int jmpMethod) {
    enum InjectStatus ijsStatus;
    struct TargetInfo ti;

    char buffer[100];
    char mem[] = "mem";

    snprintf(buffer, sizeof(buffer), "%s/%s", target, mem);
    
    int fd = open(buffer, O_RDWR);

    if (fd == -1) {
        switch (errno) {
        case ENOENT:
            return IJS_TARGET_NOT_FOUND;
        case EACCES:
        case EPERM:
            return IJS_ACCESS_DENIED;
        default:
            return IJS_OPEN_ERROR;
        }
    }

    if (saveTargetInfo(fd, &ti, target)) {
        return IJS_PARSE_ERROR;
    }

    if (injectPayload(fd, &ti, payload)) {
        return IJS_INJECT_ERROR;
    }
    
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

    close(fd);
    return ijsStatus;
}



