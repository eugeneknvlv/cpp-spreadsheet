#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

namespace {
    using namespace std::literals;
    const std::string uppercase_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"s;
    const std::string digits = "0123456789"s;
    bool IsValidSV(std::string_view cell_sv) {
        bool contains_only_admissible_symbols = cell_sv.find_first_not_of(uppercase_alphabet + digits) == cell_sv.npos;
        bool contains_uppercase_letters = cell_sv.find_first_not_of(uppercase_alphabet) != cell_sv.npos;
        bool contains_digits = cell_sv.find_first_not_of(digits) != cell_sv.npos;
        bool letters_first = cell_sv.find_last_of(uppercase_alphabet) < cell_sv.find_first_of(digits);

        return 
            contains_only_admissible_symbols &&
            contains_uppercase_letters &&
            contains_digits &&
            letters_first;
	}

    std::string_view GetRowSV(std::string_view cell_sv) {
		return cell_sv.substr(cell_sv.find_first_of(digits));
	}

	std::string_view GetColSV(std::string_view cell_sv) {
		return cell_sv.substr(0, cell_sv.find_first_of(digits));
	}
}

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

std::string Position::ToString() const {
    if (!IsValid()) {
        return "";
    }

    std::string result;
    result.reserve(MAX_POSITION_LENGTH);
    int c = col;
    while (c >= 0) {
        result.insert(result.begin(), 'A' + c % LETTERS);
        c = c / LETTERS - 1;
    }

    result += std::to_string(row + 1);

    return result;
}

Position Position::FromString(std::string_view str) {
    if (!IsValidSV(str)) {
        return Position::NONE;
    }

    auto letters = GetColSV(str);
    auto digits = GetRowSV(str);

    int col = 0;
    for (char ch : letters) {
        col *= LETTERS;
        col += ch - 'A' + 1;
    }
    
    int row; 
    try {
        row = std::stoi(std::string(digits));
    }
    catch (...) {
        return Position::NONE;
    }

    Position result = {row - 1, col - 1};
    if (!result.IsValid()) {
        return Position::NONE;
    }
    return result;
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}

FormulaError::FormulaError(Category category)
    : category_(category)
{}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    if (category_ == Category::Ref) {
        return "#REF!";
    } 
    else if (category_ == Category::Value) {
        return "#VALUE!";
    } 
    else {
        return "#DIV/0!";
    }
}