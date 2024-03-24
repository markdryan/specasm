.TestFunction
  call Function
  or a
  jr z, good
  ld bc, 1
  ret
.good
  ld bc, 0
  ret
