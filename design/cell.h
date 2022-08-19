#pragma once

#include "common.h"
#include "formula.h"

class Impl;
class EmptyImpl;
class TextImpl;
class FormulaImpl;

class Cell : public CellInterface {
public:
    Cell(const Sheet& sheet);
    Cell(Cell&& other);
    Cell(Cell& other) = delete;
    Cell(const Cell& other) = delete;
    ~Cell(); 

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    bool IsEmpty() const;

    std::vector<Position> GetReferencedCells() const override;

private:
    std::unique_ptr<Impl> impl_;
    const Sheet& sheet_; // необходимо для работы FormulaInteface::Evaluate
};

class Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;

    template <typename T>
    T* TryAs() const {
        return dynamic_cast<T*>(this);
    }
};

class EmptyImpl: public Impl {
public:
    explicit EmptyImpl() = default;
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
};

class TextImpl: public Impl {
public:
    explicit TextImpl(std::string text);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;

private:
    std::string text_;
};

class FormulaImpl: public Impl {
public:
    explicit FormulaImpl(std::string expression);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
    bool IsCached() const;
    void InvalidateCache();
    std::optional<CellInterface::Value> GetCachedValue() const;

private:
    std::unique_ptr<FormulaInterface> formula_;
    const Sheet& sheet_; // Необходимо для работы Evaluate
    std::optional<CellInterface::Value> cached_value_;
};