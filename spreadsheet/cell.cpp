#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell()
    : impl_(new EmptyImpl())
    , sheet_ptr_(nullptr)
{}

Cell::Cell(const SheetInterface* sheet_ptr) 
    : impl_(new EmptyImpl())
    , sheet_ptr_(sheet_ptr)
{}

Cell::Cell(Cell&& other)
    : impl_(std::move(other.impl_))
    , sheet_ptr_(other.sheet_ptr_)
{}

Cell::~Cell() = default;

void Cell::Set(std::string text, const SheetInterface* sheet_ptr) {
    sheet_ptr_ = sheet_ptr;
    if (text.empty()) {
        Clear();
        type_ = Empty;
        return;
    }

    if (text[0] == FORMULA_SIGN) {
        if (text.length() > 1) {
            impl_.reset(new FormulaImpl(text.substr(1), sheet_ptr_));
            type_ = Formula;
        }
        else {
            impl_.reset(new TextImpl(std::move(text)));
            type_ = Text;
        }
    }
    else {
        impl_.reset(new TextImpl(std::move(text)));
        type_ = Text;
    }
}

void Cell::Clear() {
    impl_.reset(new EmptyImpl());
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

void Cell::InvalidateCache() {
    // TODO
}

Cell::Type Cell::GetType() const {
    return type_;
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

CellInterface::Value EmptyImpl::GetValue() const {
    using namespace std::literals;
    return ""s;
}

std::string EmptyImpl::GetText() const {
    using namespace std::literals;
    return ""s;
}

bool EmptyImpl::IsCached() const {
    return true;
}

void EmptyImpl::InvalidateCache() {
    // nothing to do
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    return {};
}

TextImpl::TextImpl(std::string text)
    : text_(std::move(text))
{}

CellInterface::Value TextImpl::GetValue() const {
    if (text_[0] == '\'') {
        return text_.substr(1);
    }
    return text_;
}

std::string TextImpl::GetText() const {
    return text_;
}

bool TextImpl::IsCached() const {
    return true;
}

void TextImpl::InvalidateCache() {
    // nothing to do
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    return {};
}

FormulaImpl::FormulaImpl(std::string expression, const SheetInterface* sheet_ptr)
    : formula_(ParseFormula(expression))
    , sheet_ptr_(sheet_ptr)
{}

CellInterface::Value FormulaImpl::GetValue() const {
    if (IsCached()) {
        return *cached_value_;
    }
    FormulaInterface::Value res = formula_->Evaluate(*sheet_ptr_);
    if (std::holds_alternative<double>(res)) {
        cached_value_ = std::get<double>(res);
    }
    else {
        cached_value_ = std::get<FormulaError>(res);
    }
    return *cached_value_;
}

std::string FormulaImpl::GetText() const {
    using namespace std::literals;
    return "="s + formula_->GetExpression();
}

bool FormulaImpl::IsCached() const {
    return cached_value_.has_value();
}

void FormulaImpl::InvalidateCache() {
    cached_value_.reset();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}