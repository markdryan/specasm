org $8000
.Main
  jr near    ; 0
ds 128, 0    ; 2
.near        ; 132
  ret
.near
db 'a'
