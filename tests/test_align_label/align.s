org $8037
.Main
ld hl, label
ld hl, Global
ret
align 256
 ; 201	
.label
db $66
jr label ; 202
	

