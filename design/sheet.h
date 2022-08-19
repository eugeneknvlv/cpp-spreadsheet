#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

using CellsMatrix = std::vector<std::vector<Cell>>;

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
private:
	CellsMatrix sheet_;
    std::map<int, int> row_to_cell_count_;
    std::map<int, int> col_to_cell_count_; 

    // Структура: ключ - позиция некоторой формульной ячейки,
    // значение - множество ячеек, непосредственно зависящих от данной.
    std::unordered_map<Position, std::unordered_set<Position>> dependencies_;

    void ResizeSheetIfNeeded(Position new_cell_pos);
    void AdjustMapsAfterErasing(Position erased_pos);
    void ResizeNewlyCreatedRows(size_t old_size);

    // Возвращает true, если формульная ячейка на позиции new_cell_pos
    // содержит циклические зависимости.
    // Метод SetCell на основании результата работы этого метода будет
    // либо выбрасывать исключение CircularDependencyException,
    // либо продолжать работу
    bool CheckNoCircularDependencies(Position new_cell_pos);
    void InvalidateCache(Position pos);
};

std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value);