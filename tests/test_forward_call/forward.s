org $8000
.Main
  call far     ; 0
  ret          ; 3
ds 300, 0      ; 4
.far           ; 307
  ret
