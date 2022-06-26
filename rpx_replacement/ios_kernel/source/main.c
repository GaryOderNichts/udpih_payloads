#include "imports.h"
#include "../../ios_mcp/ios_mcp_syms.h"

extern char __kernel_bss_start;
extern char __kernel_bss_end;

int _main(void* arg)
{
    int level = disable_interrupts();
    uint32_t control_register = disable_mmu();

    // clear all bss
    memset(&__kernel_bss_start, 0, &__kernel_bss_end - &__kernel_bss_start);
    memset((void*) (__mcp_bss_start - 0x05074000 + 0x08234000), 0, __mcp_bss_end - __mcp_bss_start);

    // map the mcp sections
    ios_map_shared_info_t map_info;
    map_info.paddr  = 0x050bd000 - 0x05000000 + 0x081c0000;
    map_info.vaddr  = 0x050bd000;
    map_info.size   = 0x3000;
    map_info.domain = 1; // MCP
    map_info.type   = 3;
    map_info.cached = 0xffffffff;
    _iosMapSharedUserExecution(&map_info);

    map_info.paddr  = 0x05116000 - 0x05100000 + 0x13d80000;
    map_info.vaddr  = 0x05116000;
    map_info.size   = 0x4000;
    map_info.domain = 1; // MCP
    map_info.type   = 3;
    map_info.cached = 0xffffffff;
    _iosMapSharedUserExecution(&map_info);

    // replace the next loaded rpx
    *(volatile uint32_t *) (0x050254D6 - 0x05000000 + 0x081C0000) = THUMB_BL(0x050254D6, MCP_LoadFile_patch);

    // cos.xml access patches
    *(volatile uint32_t *) (0x0501dd78 - 0x05000000 + 0x081C0000) = THUMB_BL(0x0501dd78, MCP_ReadCOSXml_patch);
    *(volatile uint32_t *) (0x051105ce - 0x05000000 + 0x081C0000) = THUMB_BL(0x051105ce, MCP_ReadCOSXml_patch);

    restore_mmu(control_register);

    // invalidate all cache
    invalidate_dcache(NULL, 0x4001);
    invalidate_icache();

    enable_interrupts(level);

    return 0;
}
