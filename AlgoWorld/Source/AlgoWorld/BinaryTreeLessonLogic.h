#pragma once
#include <vector>

// Independent of Unreal: breadth-first array layout; -1 denotes an empty slot.
namespace AlgoTreeLesson
{
inline void Walk(const std::vector<int>& Values, int Index, int Mode, std::vector<int>& Out)
{
    if (Index < 0 || Index >= static_cast<int>(Values.size()) || Values[Index] == -1) return;
    if (Mode == 0) Out.push_back(Index);
    Walk(Values, 2 * Index + 1, Mode, Out);
    if (Mode == 1) Out.push_back(Index);
    Walk(Values, 2 * Index + 2, Mode, Out);
    if (Mode == 2) Out.push_back(Index);
}
inline std::vector<int> Order(const std::vector<int>& Values, int Mode)
{
    std::vector<int> Result;
    if (Mode < 0 || Mode > 3 || Values.empty() || Values[0] == -1) return Result;
    if (Mode != 3) { Walk(Values, 0, Mode, Result); return Result; }
    Result.push_back(0);
    for (unsigned int Cursor = 0; Cursor < Result.size(); ++Cursor)
        for (int Child : {2 * Result[Cursor] + 1, 2 * Result[Cursor] + 2})
            if (Child < static_cast<int>(Values.size()) && Values[Child] != -1) Result.push_back(Child);
    return Result;
}
}
