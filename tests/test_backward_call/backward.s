org $8000
.Main
  ds 256, 0      ; 0
  ret            ; 256
  call nz, Main  ; 257
  
