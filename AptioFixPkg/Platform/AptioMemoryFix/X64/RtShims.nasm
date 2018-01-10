;------------------------------------------------------------------------------
;
; Functions to wrap RuntimeServices code that needs to be written to
;
; by Download-Fritz & vit9696
;------------------------------------------------------------------------------

BITS     64
DEFAULT  REL

SECTION .text

global ASM_PFX(gRtShimsDataStart)
ASM_PFX(gRtShimsDataStart):

global ASM_PFX(RtShimSetVariable)
ASM_PFX(RtShimSetVariable):
    push       rsi
    push       rbx
    sub        rsp, 0x38
    pushfq
    pop        rsi
    cli
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [rsp+0x70]
    mov        qword [rsp+0x20], rax
    mov        rax, qword [ASM_PFX(gSetVariable)]
    call       rax
    test       ebx, 0x10000
    je         SET_SKIP_RESTORE_WP
    mov        cr0, rbx
SET_SKIP_RESTORE_WP:
    test       esi, 0x200
    je         SET_SKIP_RESTORE_INTR
    sti
SET_SKIP_RESTORE_INTR:
    add        rsp, 0x38
    pop        rbx
    pop        rsi
    ret

global ASM_PFX(RtShimGetVariable)
ASM_PFX(RtShimGetVariable):
    push       rsi
    push       rbx
    sub        rsp, 0x38
    pushfq
    pop        rsi
    cli
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [rsp+0x70]
    mov        qword [rsp+0x20], rax
    ; Until boot.efi virtualizes the pointers we use a custom wrapper.
    mov        rax, qword [ASM_PFX(gGetVariableBoot)]
    test       rax, rax
    jnz        GET_USE_CURRENT_FUNC
    mov        rax, qword [ASM_PFX(gGetVariable)]
GET_USE_CURRENT_FUNC:
    call       rax
    test       ebx, 0x10000
    je         GET_SKIP_RESTORE_WP
    mov        cr0, rbx
GET_SKIP_RESTORE_WP:
    test       esi, 0x200
    je         GET_SKIP_RESTORE_INTR
    sti
GET_SKIP_RESTORE_INTR:
    add        rsp, 0x38
    pop        rbx
    pop        rsi
    ret

global ASM_PFX(RtShimGetNextVariableName)
ASM_PFX(RtShimGetNextVariableName):
    nop
    push       rsi
    push       rbx
    sub        rsp, 0x28
    pushfq
    pop        rsi
    cli
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [ASM_PFX(gGetNextVariableName)]
    call       rax
    test       ebx, 0x10000
    je         NEXT_SKIP_RESTORE_WP
    mov        cr0, rbx
NEXT_SKIP_RESTORE_WP:
    test       esi, 0x200
    je         NEXT_SKIP_RESTORE_INTR
    sti
NEXT_SKIP_RESTORE_INTR:
    add        rsp, 0x28
    pop        rbx
    pop        rsi
    ret

global ASM_PFX(gGetNextVariableName)
ASM_PFX(gGetNextVariableName):    dq  0

global ASM_PFX(gGetVariable)
ASM_PFX(gGetVariable):            dq  0

global ASM_PFX(gSetVariable)
ASM_PFX(gSetVariable):            dq  0

global ASM_PFX(gGetVariableBoot)
ASM_PFX(gGetVariableBoot):        dq  0

global ASM_PFX(gRtShimsDataEnd)
ASM_PFX(gRtShimsDataEnd):
