org &8000
.Main
  jp far     ; 0
  jr near    ; 3
repb 0, 126  ; 5
.near        ; 131
ret          ; 131
repb 0, 256  ; 132
.far         ; 388
ret
