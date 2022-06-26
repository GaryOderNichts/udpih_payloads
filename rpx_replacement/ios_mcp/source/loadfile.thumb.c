#include "imports.h"
#include "fsa.h"
#include "loadfile.h"

#include <string.h>

#define real_MCP_LoadFile ((int (*)(ipcmessage *msg)) 0x0501CAA8 + 1) // + 1 for thumb
#define MCP_DoLoadFile    ((int (*)(const char *path, const char *path2, void *outputBuffer, uint32_t outLength, uint32_t pos, int *bytesRead, uint32_t unk)) 0x05017248 + 1)
#define real_MCP_ReadCOSXml_patch ((int (*)(uint32_t u1, uint32_t u2, MCPPPrepareTitleInfo *xmlData)) 0x050024ec + 1)

static int MCP_LoadCustomFile(void *buffer_out, int buffer_len, int pos)
{
    int fsaFd = IOS_Open("/dev/fsa", 0);
    FSA_Mount(fsaFd, "/dev/sdcard01", "/vol/storage_homebrew", 2, NULL, 0);
    IOS_Close(fsaFd);

    int bytesRead = 0;
    int result = MCP_DoLoadFile("/vol/storage_homebrew/launch.rpx", NULL, buffer_out, buffer_len, pos, &bytesRead, 0);
    if (result >= 0) {
        if (!bytesRead) {
            return 0;
        }
        if (result >= 0) {
            return bytesRead;
        }
    }
    return result;
}

static int _strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s);

    return (s - str);
}

int MCP_LoadFile_patch(ipcmessage *msg)
{
    MCPLoadFileRequest *request = (MCPLoadFileRequest *) msg->ioctl.buffer_in;

    if(request->name[_strlen(request->name) - 1] == 'x') {
        // replace the menu
        //if (strncmp(request->name + (_strlen(request->name) - 7), "men.rpx", sizeof("men.rpx")) == 0) {
            return MCP_LoadCustomFile(msg->ioctl.buffer_io, msg->ioctl.length_io, request->pos);
        //}
    }

    return real_MCP_LoadFile(msg);
}

int MCP_ReadCOSXml_patch(uint32_t u1, uint32_t u2, MCPPPrepareTitleInfo *xmlData)
{
    int res = real_MCP_ReadCOSXml_patch(u1, u2, xmlData);

    // give title full permission for everything
    for (uint32_t i = 0; i < 19; i++) {                    
        xmlData->permissions[i].mask = 0xFFFFFFFFFFFFFFFF;
    }

    // allow codegen access
    xmlData->codegen_size = 0x02000000;
    xmlData->codegen_core = 0x80000001;

    return res;
}
