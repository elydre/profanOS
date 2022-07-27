global switchTask

switchTask:
    pusha
    pushf
    mov    eax, cr3
    push   eax
    mov    eax, [esp+0x2c]
    mov    [eax+0x4], ebx
    mov    [eax+0x8], ecx
    mov    [eax+0xc], edx
    mov    [eax+0x10], esi
    mov    [eax+0x14], edi
    mov    ebx, [esp+0x24]
    mov    ecx, [esp+0x28]
    mov    edx, [esp+0x14]
    add    edx, 0x4
    mov    esi, [esp+0x10]
    mov    edi, [esp+0x4]
    mov    [eax], ebx
    mov    [eax+0x18], edx
    mov    [eax+0x1c], esi
    mov    [eax+0x20], ecx
    mov    [eax+0x24], edi
    pop    ebx
    mov    [eax+0x28], ebx
    push   ebx
    mov    eax, [esp+0x30]
    mov    ebx, [eax+0x4]
    mov    ecx, [eax+0x8]
    mov    edx, [eax+0xc]
    mov    esi, [eax+0x10]
    mov    edi, [eax+0x14]
    mov    ebp, [eax+0x1c]
    push   eax
    mov    eax, [eax+0x24]
    push   eax
    popf
    pop    eax
    mov    esp, [eax+0x18]
    push   eax
    mov    eax, [eax+0x28]
    mov    cr3, eax
    pop    eax
    push   eax
    mov    eax, [eax+0x20]
    xchg   [esp], eax
    mov    eax, [eax]
    ret
