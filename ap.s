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
	hlt
	jmp $

msg:
	db "salut depuis le coeur1", 0x10, 0
