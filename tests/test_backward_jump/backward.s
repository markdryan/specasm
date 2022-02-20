org &8000
.Main
  repb 0, 256  ; 0
  ret          ; 256
.near          ; 257
repb 0, 122    ; 257
  ret          ; 379
  jp nz, Main  ; 380
  jr near      ; 383
  
