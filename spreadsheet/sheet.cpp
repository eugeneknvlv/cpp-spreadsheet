#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }
    ResizeSheetIfNeeded(pos);
    ProcessCellSetting(std::move(pos), std::move(text));
    InvalidateCache(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!CheckPositionCorrectness(pos)) {
        return nullptr;
    }
    return &sheet_[pos.row][pos.col];
}

void Sheet::ClearCell(Position pos) {
    if (!CheckPositionCorrectness(pos)) {
        return;
    }
    sheet_[pos.row][pos.col].Clear();
    AdjustMapsAfterErasing(pos);
}

Size Sheet::GetPrintableSize() const {
    if (row_to_cell_count_.empty() || col_to_cell_count_.empty()) {
        return {0, 0};
    }

    return 
    { 
        prev(row_to_cell_count_.end())->first + 1, 
        prev(col_to_cell_count_.end())->first + 1 
    };
}

void Sheet::PrintValues(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for (int row = 0; row < printable_size.rows; ++row) {
        for (int col = 0; col < printable_size.cols; ++col) {
            output << (col > 0 ? "\t" : "") << sheet_[row][col].GetValue();
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for (int row = 0; row < printable_size.rows; ++row) {
        for (int col = 0; col < printable_size.cols; ++col) {
            output << (col > 0 ? "\t" : "") << sheet_[row][col].GetText();
        }
        output << '\n';
    }
}

void Sheet::ResizeSheetIfNeeded(Position new_cell_pos) {
    if (new_cell_pos.row >= static_cast<int>(sheet_.size())) {
        size_t old_size = sheet_.size();
        sheet_.resize(new_cell_pos.row + 1);
        ResizeNewlyCreatedRows(old_size);
    }
    if (new_cell_pos.col >= static_cast<int>(sheet_[0].size())) {
        for (auto& row : sheet_) {
            row.resize(new_cell_pos.col + 1);
        }
    }
}

void Sheet::AdjustMapsAfterErasing(Position erased_pos) {
    if (row_to_cell_count_.count(erased_pos.row)) {
        --row_to_cell_count_[erased_pos.row];
        if (row_to_cell_count_[erased_pos.row] == 0) {
            row_to_cell_count_.erase(erased_pos.row);
        }
    }
    if (col_to_cell_count_.count(erased_pos.col)) {
        --col_to_cell_count_[erased_pos.col];
        if (col_to_cell_count_[erased_pos.col] == 0) {
            col_to_cell_count_.erase(erased_pos.col);
        }
    }
}

void Sheet::ResizeNewlyCreatedRows(size_t old_size) {
    for (size_t i = old_size; i < sheet_.size(); ++i) {
        sheet_[i].resize(sheet_[0].size());
    }
}

void Sheet::ProcessCellSetting(Position pos, std::string text) {
    std::string old_cell_state = GetCell(pos)->GetText();
    sheet_[pos.row][pos.col].Set(text, this);
    if (sheet_[pos.row][pos.col].GetType() == Cell::Type::Formula) {
        auto old_dependencies_state = dependencies_;
        for (const Position& referenced_pos : sheet_[pos.row][pos.col].GetReferencedCells()) {
            dependencies_[referenced_pos].insert(pos);
        }
        if (IsCircularDependent(pos, pos)) {
            dependencies_ = old_dependencies_state;
            SetCell(pos, old_cell_state);
            throw CircularDependencyException("Circular dependency"s);
        }
    }
    ++row_to_cell_count_[pos.row];
    ++col_to_cell_count_[pos.col];
}

bool Sheet::CheckPositionCorrectness(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }
    if (sheet_.size() == 0) {
        return false;
    }
    if (pos.row >= static_cast<int>(sheet_.size()) || pos.col >= static_cast<int>(sheet_[0].size())) {
        return false;
    }
    return true;
}

void Sheet::InvalidateCache(Position pos) {
    GetCell(pos)->InvalidateCache();
    if (!dependencies_.count(pos)) {
        return;
    }
    for (const Position& pos : dependencies_.at(pos)) {
        InvalidateCache(pos);
    }
}

bool Sheet::IsCircularDependent(Position pos, Position initial_pos) {
    if (dependencies_.count(pos) == 0) {
        return false;
    }
    if (dependencies_.at(pos).count(initial_pos)) {
        return true;
    }
    for (const Position& referenced_pos : dependencies_.at(pos)) {
        if (!dependencies_.count(referenced_pos)) {
            continue;
        }
        if (IsCircularDependent(referenced_pos, initial_pos)) {
            return true;
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    if (std::holds_alternative<std::string>(value)) {
        output << std::get<std::string>(value);
    }
    else if (std::holds_alternative<double>(value)) {
        output << std::get<double>(value);
    }
    else {
        output << std::get<FormulaError>(value);
    }
    return output;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}