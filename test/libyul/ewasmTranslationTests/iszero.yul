{
  let x := 0
  let y := 0x0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20
  let z := 0x0000000000000000000111111111111111111111100000000000000000000000
  sstore(0, iszero(x))
  sstore(1, iszero(y))
  sstore(2, iszero(z))
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000002
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000000: 0000000000000000000000000000000000000000000000000000000000000001
