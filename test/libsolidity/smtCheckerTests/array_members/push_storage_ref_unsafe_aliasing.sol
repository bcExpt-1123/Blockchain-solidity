pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[0].push();
		a[0][0] = 16;
		uint[] storage b = a[0];
		b[0] = 32;
		// Access is safe but fails due to aliasing.
		assert(a[0][0] == 16);
	}
}
// ----
// Warning 6368: (221-225): CHC: Out of bounds access happens here.\nCounterexample:\na = []\nb = [32]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
// Warning 6368: (221-228): CHC: Out of bounds access happens here.\nCounterexample:\na = [[], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15], [15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15]]\nb = [32]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
// Warning 6328: (214-235): CHC: Assertion violation happens here.\nCounterexample:\n\nb = [32]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
