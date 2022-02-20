org &8000
.Main
  jr near    ; 0
repb 0, 128  ; 2
.near        ; 132
  ret
.near
equb 'a'	
