/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <test/libyul/EWasmTranslationTest.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/Options.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/wasm/EVMToEWasmTranslator.h>
#include <libyul/AsmParser.h>
#include <libyul/AssemblyStack.h>
#include <libyul/AsmAnalysisInfo.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libdevcore/AnsiColorized.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace dev;
using namespace langutil;
using namespace yul;
using namespace yul::test;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace std;


EWasmTranslationTest::EWasmTranslationTest(string const& _filename)
{
	boost::filesystem::path path(_filename);

	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test case: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSourceAndSettings(file);
	m_expectation = parseSimpleExpectations(file);
}

TestCase::TestResult EWasmTranslationTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	if (!parse(_stream, _linePrefix, _formatted))
		return TestResult::FatalError;

	*m_object = EVMToEWasmTranslator(
		EVMDialect::strictAssemblyForEVMObjects(dev::test::Options::get().evmVersion())
	).run(*m_object);

	// Add call to "main()".
	m_object->code->statements.emplace_back(
		ExpressionStatement{{}, FunctionCall{{}, Identifier{{}, "main"_yulstring}, {}}}
	);

	m_obtainedResult = interpret();

	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		// TODO could compute a simple diff with highlighted lines
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return TestResult::Failure;
	}
	return TestResult::Success;
}

void EWasmTranslationTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void EWasmTranslationTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void EWasmTranslationTest::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		_stream << _linePrefix << line << endl;
}

bool EWasmTranslationTest::parse(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	AssemblyStack stack(
		dev::test::Options::get().evmVersion(),
		AssemblyStack::Language::StrictAssembly,
		dev::solidity::OptimiserSettings::none()
	);
	if (stack.parseAndAnalyze("", m_source))
	{
		m_object = stack.parserResult();
		return true;
	}
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, stack.errors());
		return false;
	}
}

string EWasmTranslationTest::interpret()
{
	InterpreterState state;
	state.maxTraceSize = 10000;
	state.maxSteps = 100000;
	WasmDialect dialect;
	Interpreter interpreter(state, dialect);
	try
	{
		interpreter(*m_object->code);
	}
	catch (InterpreterTerminatedGeneric const&)
	{
	}

	stringstream result;
	state.dumpTraceAndState(result);
	return result.str();
}

void EWasmTranslationTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printErrorInformation(*error);
}
