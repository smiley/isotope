#include "Utilities.h"

#include <codecvt>
#include <locale>
#include <string>

#include "Types.h"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

std::wstring toWide(const std::string& original) {
    return converter.from_bytes(original);
}

DWORD GetOverlappedResults(HANDLE file, LPOVERLAPPED overlapped) {
    DWORD result = 0;

    if (GetOverlappedResult(file, overlapped, &result, FALSE) == FALSE) {
        if (GetLastError() == ERROR_IO_PENDING) {
            return -1;
        }
        throw WinAPIException(GetLastError());
    }

    return result;
}

std::string toNarrow(const std::wstring& original) {
    return converter.to_bytes(original);
}