[org 0x7000]
[bits 16]

SERIAL_PORT equ 0x3f8

mov bx, msg
loop:


_wait:
    mov dx, SERIAL_PORT + 5
    in al, dx
    and al, 0x20
    cmp al, 0
    je _wait

    mov dx, SERIAL_PORT
    mov al, [bx]
    out dx, al

	inc bx

	cmp byte [bx], 0
	je out

    jmp loop

out:
	jmp start

msg:
	db "salut depuis le coeur1", 0x10, 0

PS2_DATA   equ 0x60
PS2_STATUS equ 0x64

start:

.wait1:
    in  al, PS2_STATUS
    test al, 2
    jnz .wait1

    mov al, 0xED
    out PS2_DATA, al

.wait_ack1:
    in  al, PS2_STATUS
    test al, 1
    jz .wait_ack1

    in  al, PS2_DATA
    cmp al, 0xFA
    jne $

.wait2:
    in  al, PS2_STATUS
    test al, 2
    jnz .wait2

    mov al, 0x04
    out PS2_DATA, al

.wait_ack2:
    in  al, PS2_STATUS
    test al, 1
    jz .wait_ack2

    in  al, PS2_DATA
    cmp al, 0xFA
    jne $

hang:
	hlt
    jmp hang
