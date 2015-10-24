#pragma once

#include <windows.h>
#include <Shlobj.h>

#include <string>
#include <vector>

std::vector<std::wstring> GetDrivesOfType(DWORD driveType);

bool isDiscInDrive(const std::wstring& drive);

std::wstring GetDriveName(const std::wstring& drive);

std::wstring ResolveShellItem(IShellItem* item);