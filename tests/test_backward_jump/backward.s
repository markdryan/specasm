org $8000
.Main
  ds 256, 0    ; 0
  ret          ; 256
; 257	
.near
ds 122, 0      ; 257
  ret          ; 379
  jp nz, Main  ; 380
  jr near      ; 383
  
