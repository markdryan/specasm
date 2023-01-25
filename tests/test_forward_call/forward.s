org $8000
.Main
  call far     ; 0
  ret          ; 3
  ds 300, 0      ; 4
; 307	
.far
  ret
