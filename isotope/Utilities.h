#pragma once

#include <Windows.h>

#include <memory>
#include <string>

std::string toNarrow(const std::wstring& original);
std::wstring toWide(const std::string& original);

DWORD GetOverlappedResults(HANDLE file, LPOVERLAPPED overlapped);

struct FreeDeleter : std::default_delete<void> {
    void operator()(void* item) {
        free(item);
    }
};