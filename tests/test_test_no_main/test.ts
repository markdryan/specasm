.Main
map
.TestGlobal
  ld bc, 1
  call Global
  ret c
  ld bc, 0
  ret
