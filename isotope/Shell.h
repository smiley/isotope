#pragma once

#include <windows.h>
#include <Shlobj.h>

#include <string>
#include <vector>

class CoInitializer {
public:
    CoInitializer() {
        CoInitialize(nullptr);
    }

    ~CoInitializer() {
        CoUninitialize();
    }

private:
    CoInitializer& operator=(const CoInitializer& other);
    CoInitializer(const CoInitializer& other);
    CoInitializer(CoInitializer&& other);
};

std::vector<std::wstring> GetDrivesOfType(DWORD driveType);

bool isDiscInDrive(const std::wstring& drive);

std::wstring GetDriveName(const std::wstring& drive);

std::wstring ResolveShellItem(IShellItem* item);

IShellItem* CreateShellItemFromPath(const std::wstring& path);

void OpenExplorerOnFile(const std::wstring& path);

namespace Path {
    std::wstring GetParentFolder(const std::wstring& path);
}