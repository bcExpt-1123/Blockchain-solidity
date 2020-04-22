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
/**
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 */

#include <libyul/optimiser/Suite.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/CircularReferencesPruner.h>
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopConditionOutOfBody.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/LoadResolver.h>
#include <libyul/optimiser/LoopInvariantCodeMotion.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/backends/evm/ConstantOptimiser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Object.h>

#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/evm/NoOutputAssembly.h>

#include <libsolutil/CommonData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void OptimiserSuite::run(
	Dialect const& _dialect,
	GasMeter const* _meter,
	Object& _object,
	bool _optimizeStackAllocation,
	set<YulString> const& _externallyUsedIdentifiers
)
{
	set<YulString> reservedIdentifiers = _externallyUsedIdentifiers;
	reservedIdentifiers += _dialect.fixedFunctionNames();

	*_object.code = std::get<Block>(Disambiguator(
		_dialect,
		*_object.analysisInfo,
		reservedIdentifiers
	)(*_object.code));
	Block& ast = *_object.code;

	OptimiserSuite suite(_dialect, reservedIdentifiers, Debug::None, ast);

	suite.runSequence(
		"dhfoDgvulfnTUtnIf"            // None of these can make stack problems worse
		"("
			"xarrscLM"                 // Turn into SSA and simplify
			"cCTUtTOntnfDIul"          // Perform structural simplification
			"Lcul"                     // Simplify again
			"Vcul jj"                  // Reverse SSA

			// should have good "compilability" property here.

			"eul"                      // Run functional expression inliner
			"xarulrul"                 // Prune a bit more in SSA
			"xarrcL"                   // Turn into SSA again and simplify
			"gvif"                     // Run full inliner
			"CTUcarrLsTOtfDncarrIulc"  // SSA plus simplify
		")"
		"jmuljuljul VcTOcul jmul",     // Make source short and pretty
		ast
	);

	// This is a tuning parameter, but actually just prevents infinite loops.
	size_t stackCompressorMaxIterations = 16;
	suite.runSequence("g", ast);

	// We ignore the return value because we will get a much better error
	// message once we perform code generation.
	StackCompressor::run(
		_dialect,
		_object,
		_optimizeStackAllocation,
		stackCompressorMaxIterations
	);
	suite.runSequence("fDnTOc g", ast);

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&_dialect))
	{
		yulAssert(_meter, "");
		ConstantOptimiser{*dialect, *_meter}(ast);
	}
	else if (dynamic_cast<WasmDialect const*>(&_dialect))
	{
		// If the first statement is an empty block, remove it.
		// We should only have function definitions after that.
		if (ast.statements.size() > 1 && std::get<Block>(ast.statements.front()).statements.empty())
			ast.statements.erase(ast.statements.begin());
	}
	VarNameCleaner::run(suite.m_context, ast);

	*_object.analysisInfo = AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, _object);
}

namespace
{


template <class... Step>
map<string, unique_ptr<OptimiserStep>> optimiserStepCollection()
{
	map<string, unique_ptr<OptimiserStep>> ret;
	for (unique_ptr<OptimiserStep>& s: util::make_vector<unique_ptr<OptimiserStep>>(
		(make_unique<OptimiserStepInstance<Step>>())...
	))
	{
		yulAssert(!ret.count(s->name), "");
		ret[s->name] = std::move(s);
	}
	return ret;
}

}

map<string, unique_ptr<OptimiserStep>> const& OptimiserSuite::allSteps()
{
	static map<string, unique_ptr<OptimiserStep>> instance;
	if (instance.empty())
		instance = optimiserStepCollection<
			BlockFlattener,
			CircularReferencesPruner,
			CommonSubexpressionEliminator,
			ConditionalSimplifier,
			ConditionalUnsimplifier,
			ControlFlowSimplifier,
			DeadCodeEliminator,
			EquivalentFunctionCombiner,
			ExpressionInliner,
			ExpressionJoiner,
			ExpressionSimplifier,
			ExpressionSplitter,
			ForLoopConditionIntoBody,
			ForLoopConditionOutOfBody,
			ForLoopInitRewriter,
			FullInliner,
			FunctionGrouper,
			FunctionHoister,
			LiteralRematerialiser,
			LoadResolver,
			LoopInvariantCodeMotion,
			RedundantAssignEliminator,
			Rematerialiser,
			SSAReverser,
			SSATransform,
			StructuralSimplifier,
			UnusedPruner,
			VarDeclInitializer
		>();
	// Does not include VarNameCleaner because it destroys the property of unique names.
	return instance;
}

map<string, char> const& OptimiserSuite::stepNameToAbbreviationMap()
{
	static map<string, char> lookupTable{
		{BlockFlattener::name,                'f'},
		{CircularReferencesPruner::name,      'l'},
		{CommonSubexpressionEliminator::name, 'c'},
		{ConditionalSimplifier::name,         'C'},
		{ConditionalUnsimplifier::name,       'U'},
		{ControlFlowSimplifier::name,         'n'},
		{DeadCodeEliminator::name,            'D'},
		{EquivalentFunctionCombiner::name,    'v'},
		{ExpressionInliner::name,             'e'},
		{ExpressionJoiner::name,              'j'},
		{ExpressionSimplifier::name,          's'},
		{ExpressionSplitter::name,            'x'},
		{ForLoopConditionIntoBody::name,      'I'},
		{ForLoopConditionOutOfBody::name,     'O'},
		{ForLoopInitRewriter::name,           'o'},
		{FullInliner::name,                   'i'},
		{FunctionGrouper::name,               'g'},
		{FunctionHoister::name,               'h'},
		{LiteralRematerialiser::name,         'T'},
		{LoadResolver::name,                  'L'},
		{LoopInvariantCodeMotion::name,       'M'},
		{RedundantAssignEliminator::name,     'r'},
		{Rematerialiser::name,                'm'},
		{SSAReverser::name,                   'V'},
		{SSATransform::name,                  'a'},
		{StructuralSimplifier::name,          't'},
		{UnusedPruner::name,                  'u'},
		{VarDeclInitializer::name,            'd'},
	};
	yulAssert(lookupTable.size() == allSteps().size(), "");

	return lookupTable;
}

map<char, string> const& OptimiserSuite::stepAbbreviationToNameMap()
{
	static map<char, string> lookupTable = util::invertMap(stepNameToAbbreviationMap());

	return lookupTable;
}

void OptimiserSuite::runSequence(string const& _stepAbbreviations, Block& _ast)
{
	string input = _stepAbbreviations;
	boost::remove_erase(input, ' ');
	boost::remove_erase(input, '\n');

	bool insideLoop = false;
	for (char abbreviation: input)
		switch (abbreviation)
		{
		case '(':
			assertThrow(!insideLoop, OptimizerException, "Nested parentheses not supported");
			insideLoop = true;
			break;
		case ')':
			assertThrow(insideLoop, OptimizerException, "Unbalanced parenthesis");
			insideLoop = false;
			break;
		default:
			assertThrow(
				stepAbbreviationToNameMap().find(abbreviation) != stepAbbreviationToNameMap().end(),
				OptimizerException,
				"Invalid optimisation step abbreviation"
			);
		}
	assertThrow(!insideLoop, OptimizerException, "Unbalanced parenthesis");

	auto abbreviationsToSteps = [](string const& _sequence) -> vector<string>
	{
		vector<string> steps;
		for (char abbreviation: _sequence)
			steps.emplace_back(stepAbbreviationToNameMap().at(abbreviation));
		return steps;
	};

	// The sequence has now been validated and must consist of pairs of segments that look like this: `aaa(bbb)`
	// `aaa` or `(bbb)` can be empty. For example we consider a sequence like `fgo(aaf)Oo` to have
	// four segments, the last of which is an empty parenthesis.
	size_t currentPairStart = 0;
	while (currentPairStart < input.size())
	{
		size_t openingParenthesis = input.find('(', currentPairStart);
		size_t closingParenthesis = input.find(')', openingParenthesis);
		size_t firstCharInside = (openingParenthesis == string::npos ? input.size() : openingParenthesis + 1);
		yulAssert((openingParenthesis == string::npos) == (closingParenthesis == string::npos), "");

		runSequence(abbreviationsToSteps(input.substr(currentPairStart, openingParenthesis - currentPairStart)), _ast);
		runSequenceUntilStable(abbreviationsToSteps(input.substr(firstCharInside, closingParenthesis - firstCharInside)), _ast);

		currentPairStart = (closingParenthesis == string::npos ? input.size() : closingParenthesis + 1);
	}
}

void OptimiserSuite::runSequence(std::vector<string> const& _steps, Block& _ast)
{
	unique_ptr<Block> copy;
	if (m_debug == Debug::PrintChanges)
		copy = make_unique<Block>(std::get<Block>(ASTCopier{}(_ast)));
	for (string const& step: _steps)
	{
		if (m_debug == Debug::PrintStep)
			cout << "Running " << step << endl;
		allSteps().at(step)->run(m_context, _ast);
		if (m_debug == Debug::PrintChanges)
		{
			// TODO should add switch to also compare variable names!
			if (SyntacticallyEqual{}.statementEqual(_ast, *copy))
				cout << "== Running " << step << " did not cause changes." << endl;
			else
			{
				cout << "== Running " << step << " changed the AST." << endl;
				cout << AsmPrinter{}(_ast) << endl;
				copy = make_unique<Block>(std::get<Block>(ASTCopier{}(_ast)));
			}
		}
	}
}

void OptimiserSuite::runSequenceUntilStable(
	std::vector<string> const& _steps,
	Block& _ast,
	size_t maxRounds
)
{
	size_t codeSize = 0;
	for (size_t rounds = 0; rounds < maxRounds; ++rounds)
	{
		size_t newSize = CodeSize::codeSizeIncludingFunctions(_ast);
		if (newSize == codeSize)
			break;
		codeSize = newSize;

		runSequence(_steps, _ast);
	}
}
