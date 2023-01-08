global process_asm_switch

process_asm_switch:
    pusha
    pushf
    mov    eax, cr3         ; Push CR3
    push   eax
    mov    eax, [esp+0x2c]  ; The first argument, where to save
    mov    [eax+0x4], ebx
    mov    [eax+0x8], ecx
    mov    [eax+0xc], edx
    mov    [eax+0x10], esi
    mov    [eax+0x14], edi
    mov    ebx, [esp+0x24]
    mov    ecx, [esp+0x28]
    mov    edx, [esp+0x14]
    add    edx, 0x4         ; Remove the return value 
    mov    esi, [esp+0x10]  ; EBP
    mov    edi, [esp+0x4]   ; EFLAG
    mov    [eax], ebx
    mov    [eax+0x18], edx
    mov    [eax+0x1c], esi
    mov    [eax+0x20], ecx
    mov    [eax+0x24], edi
    pop    ebx              ; CR3
    mov    [eax+0x28], ebx
    push   ebx              ; Goodbye again
    mov    eax, [esp+0x30]  ; Now it is the new object
    mov    ebx, [eax+0x4]   ; EBX
    mov    ecx, [eax+0x8]   ; ECX
    mov    edx, [eax+0xc]   ; EDX
    mov    esi, [eax+0x10]  ; ESI
    mov    edi, [eax+0x14]  ; EDI
    mov    ebp, [eax+0x1c]  ; EBP
    push   eax
    mov    eax, [eax+0x24]  ; EFLAGS
    push   eax
    popf
    pop    eax

    mov    esp, [eax+0x18]  ; pf4 est pire que fuzeIII

    push   eax
    mov    eax, [eax+0x28]  ; CR3
    mov    cr3, eax
    pop    eax
    push   eax
    mov    eax, [eax+0x20]  ; EIP
    xchg   [esp], eax
    mov    eax, [eax]
    ret                     ; This ends all

