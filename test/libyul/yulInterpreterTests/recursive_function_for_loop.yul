{
        {
            let a := foo_0(calldataload(0))
            sstore(0, a)
        }
        function foo_0(x) -> y
        {
            mstore8(1, 1)
            for {
            }
            slt(1, keccak256(1, msize()))
            {
                let x_1 := foo_0(x)
            }
            {
                continue
            }
        }
}
// ----
// Trace:
//   MSTORE_AT_SIZE(1, 1) [0101]
//   MSIZE()
//   MLOAD_FROM_SIZE(1, 32)
//   MSTORE_AT_SIZE(1, 1) [0101]
//   MSIZE()
//   MLOAD_FROM_SIZE(1, 64)
//   MSIZE()
//   MLOAD_FROM_SIZE(1, 96)
//   SSTORE(0, 0)
// Memory dump:
//      0: 0001000000000000000000000000000000000000000000000000000000000000
// Storage dump:
