org $8000
.Main
  call far     ; 0
  ret          ; 3
repb 0, 300    ; 4
.far           ; 307
  ret
