;*****************************************************************************;
;   === proc.asm : 2024 ===                                                   ;
;                                                                             ;
;    Process switch and fork entry                                 .pi0iq.    ;
;                                                                 d"  . `'b   ;
;    This file is part of profanOS and is released under          q. /|\  "   ;
;    the terms of the GNU General Public License                   `// \\     ;
;                                                                  //   \\    ;
;   === elydre : https://github.com/elydre/profanOS ===         #######  \\   ;
;*****************************************************************************;

global process_asm_switch
extern i_end_scheduler

section .data
    ; variable for eax copy
    eax_copy dd 0

align 4
process_asm_switch:
    cli                     ; disable interrupts

    pusha
    pushf
    mov    eax, cr3         ; push CR3
    push   eax
    mov    eax, [esp+0x2c]  ; the first argument, where to save
    mov    [eax+0x4], ebx
    mov    [eax+0x8], ecx
    mov    [eax+0xc], edx
    mov    [eax+0x10], esi
    mov    [eax+0x14], edi
    mov    ebx, [esp+0x24]
    mov    ecx, [esp+0x28]
    mov    edx, [esp+0x14]
    add    edx, 0x4         ; remove the return value
    mov    esi, [esp+0x10]  ; EBP
    mov    edi, [esp+0x4]   ; EFLAG
    mov    [eax], ebx
    mov    [eax+0x18], edx
    mov    [eax+0x1c], esi
    mov    [eax+0x20], ecx
    mov    [eax+0x24], edi
    pop    ebx              ; CR3
    mov    [eax+0x28], ebx
    push   ebx
    mov    eax, [esp+0x30]  ; now it is the new object
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

    ; backup eax in memory
    mov    [eax_copy], eax

    mov    eax, [eax+0x28]  ; CR3
    mov    cr3, eax

    ; restore eax
    mov    eax, [eax_copy]

    mov    esp, [eax+0x18]  ; pf4 est pire que fuzeIII
    push   eax

    pop    eax
    push   eax
    mov    eax, [eax+0x20]  ; EIP
    xchg   [esp], eax
    mov    eax, [eax]

    cld                     ; clear direction flag
    call i_end_scheduler    ; all the end of the scheduler (process.c)

    sti                     ; enable interrupts

    ret                     ; this ends all
