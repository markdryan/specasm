org $8000
.Main
  repb 0, 256    ; 0
  ret            ; 256
  call nz, Main  ; 257
  
