org $8000
.Main
  jp far     ; 0
  jr near    ; 3
ds 126, 0    ; 5
.near        ; 131
ret          ; 131
ds 256, 0    ; 132
.far         ; 388
ret
