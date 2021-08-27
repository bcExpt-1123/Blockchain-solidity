contract C {
	function f(bytes memory data) public pure {
		(uint a1, bytes32 b1, C c1) = abi.decode(data, (uint, bytes32, C));
		(uint a2, bytes32 b2, C c2) = abi.decode(data, (uint, bytes32, C));
		assert(a1 == a2);
		assert(a1 != a2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (70-80): Unused local variable.
// Warning 2072: (82-86): Unused local variable.
// Warning 2072: (140-150): Unused local variable.
// Warning 2072: (152-156): Unused local variable.
// Warning 8364: (123-124): Assertion checker does not yet implement type type(contract C)
// Warning 8364: (193-194): Assertion checker does not yet implement type type(contract C)
// Warning 6328: (220-236): CHC: Assertion violation happens here.\nCounterexample:\n\na1 = 2437\nb1 = 10\nc1 = 9\na2 = 2437\nb2 = 10\nc2 = 9\n\nTransaction trace:\nC.constructor()\nC.f(data)
