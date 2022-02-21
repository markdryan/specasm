org $8000
.Main
  ld hl, Global   		; 0
  ld hl, (Global)		; 3
  ret	       			; 6
equw Global 			; 7
	
