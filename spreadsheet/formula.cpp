#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(std::move(expression)))
    {}

    Value Evaluate(const SheetInterface& sheet) const override  { // TODO
        try {
            double tmp = ast_.Execute(sheet);
            return tmp;
        } catch (const FormulaError& exc) {
            return exc;
        }
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> result = {ast_.GetCells().begin(), ast_.GetCells().end()};
        auto new_end = std::unique(result.begin(), result.end());
        result.erase(new_end, result.end());
        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}