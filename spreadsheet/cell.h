#pragma once

#include "common.h"
#include "formula.h"
#include <optional>

class Impl;

class Cell : public CellInterface {
public:
    enum Type {
        Empty,
        Text,
        Formula
    };

    Cell();
    explicit Cell(const SheetInterface* sheet_ptr);
    Cell(Cell&& other);
    Cell(Cell& other) = delete;
    Cell(const Cell& other) = delete;
    ~Cell(); 

    void Set(std::string text, const SheetInterface* sheet_ptr_);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    void InvalidateCache() override;
    Type GetType() const;

    std::vector<Position> GetReferencedCells() const override;

private:
    std::unique_ptr<Impl> impl_;
    const SheetInterface* sheet_ptr_; // необходимо для работы FormulaInteface::Evaluate
    Type type_;
};

class Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual bool IsCached() const = 0;
    virtual void InvalidateCache() = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class EmptyImpl: public Impl {
public:
    explicit EmptyImpl() = default;
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    bool IsCached() const override;
    void InvalidateCache() override;
    std::vector<Position> GetReferencedCells() const override;
};

class TextImpl: public Impl {
public:
    explicit TextImpl(std::string text);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    bool IsCached() const override;
    void InvalidateCache() override;
    std::vector<Position> GetReferencedCells() const override;

private:
    std::string text_;
};

class FormulaImpl: public Impl {
public:
    FormulaImpl(std::string expression, const SheetInterface* sheet_ptr_);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    bool IsCached() const override;
    void InvalidateCache() override;
    std::vector<Position> GetReferencedCells() const override;

private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface* sheet_ptr_; // Необходимо для работы Evaluate
    mutable std::optional<CellInterface::Value> cached_value_;
};