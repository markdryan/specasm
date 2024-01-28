.openCh equ $1601
.printS equ 8252

.Main
	ld a, 2
	call =openCh
	ld de, data
	ld bc, (len)
	call =printS
	ret
.data
db 22, 10, 13
"SPECASM !!!"
; Store string len in 2 bytes
.len
dw =len-data
