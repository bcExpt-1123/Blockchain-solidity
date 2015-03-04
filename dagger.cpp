/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file dagger.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Dashimoto test functions.
 */

#include <fstream>
#include <random>
#include "JsonSpiritHeaders.h"
#include <libdevcore/CommonIO.h>
#include <libethcore/ProofOfWork.h>
#include <libethcore/Ethasher.h>
#include <boost/test/unit_test.hpp>
#include "TestHelper.h"

using namespace std;
using namespace dev;
using namespace dev::eth;

namespace js = json_spirit;

using dev::operator <<;

BOOST_AUTO_TEST_SUITE(DashimotoTests)

BOOST_AUTO_TEST_CASE(basic_test)
{
#if 0
	cnote << "Testing ProofOfWork...";
	// Test dagger
	{
		auto s = steady_clock::now();
		cout << hex << ProofOfWork().eval((h256)(u256)1, (h256)(u256)0);
		cout << " " << dec << duration_cast<milliseconds>(steady_clock::now() - s).count() << " ms" << endl;
		cout << hex << ProofOfWork().eval((h256)(u256)1, (h256)(u256)1);
		cout << " " << dec << duration_cast<milliseconds>(steady_clock::now() - s).count() << " ms" << endl;
	}
	{
		cnote << i.first;
		js::mObject& o = i.second.get_obj();
		vector<pair<string, string>> ss;
		BlockInfo header = BlockInfo::fromHeader(fromHex(o["header"].get_str()));
		h256 headerHash(o["header_hash"].get_str());
		Nonce nonce(o["nonce"].get_str());
		BOOST_REQUIRE_EQUAL(headerHash, header.headerHash(WithoutNonce));
		BOOST_REQUIRE_EQUAL(nonce, header.nonce);

		unsigned cacheSize(o["cache_size"].get_int());
		h256 cacheHash(o["cache_hash"].get_str());
		BOOST_REQUIRE_EQUAL(Ethasher::get()->cache(header).size(), cacheSize);
		BOOST_REQUIRE_EQUAL(sha3(Ethasher::get()->cache(header)), cacheHash);

#if TEST_FULL
		unsigned fullSize(o["full_size"].get_int());
		h256 fullHash(o["full_hash"].get_str());
		BOOST_REQUIRE_EQUAL(Ethasher::get()->full(header).size(), fullSize);
		BOOST_REQUIRE_EQUAL(sha3(Ethasher::get()->full(header)), fullHash);
#endif

		h256 result(o["result"].get_str());
		Ethasher::Result r = Ethasher::eval(header);
		BOOST_REQUIRE_EQUAL(r.value, result);
		BOOST_REQUIRE_EQUAL(r.mixHash, header.mixHash);
	}
#endif
	return 0;
}

BOOST_AUTO_TEST_SUITE_END()


