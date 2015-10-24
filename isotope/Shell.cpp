#include "Shell.h"
#include "Types.h"

#include <Windows.h>
#include <Shlobj.h>

std::vector<std::wstring> GetDrivesOfType(DWORD driveType) {
    wchar_t drivesBuffer[512] = { 0 };

    DWORD charactersCopied = GetLogicalDriveStrings(_countof(drivesBuffer), drivesBuffer);
    if (charactersCopied == 0) {
        throw WinAPIException(GetLastError());
    }

    std::vector<std::wstring> drives;

    for (unsigned int index = 0; index < charactersCopied; index += 4) {
        const wchar_t* drive = &(drivesBuffer[index]);

        if (GetDriveType(drive) == driveType) {
            drives.push_back(drive);
        }
    }

    return drives;
}

bool isDiscInDrive(const std::wstring& drive) {
    DWORD mediaType = 0;
    THROW_IF_FAILED(SHGetDriveMedia(drive.c_str(), &mediaType));

    return mediaType != ARCONTENT_NONE;
}

std::wstring GetDriveName(const std::wstring& drive) {
    wchar_t driveNameBuffer[1024] = { 0 };
    if (GetVolumeInformation(drive.c_str(), driveNameBuffer, _countof(driveNameBuffer),
                             nullptr, nullptr, nullptr, nullptr, 0) == FALSE) {
        throw WinAPIException(GetLastError());
    }

    return std::wstring(driveNameBuffer);
}

std::wstring ResolveShellItem(IShellItem* item) {
    wchar_t* filePath = nullptr;
    THROW_IF_FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath));

    std::wstring finalObject(filePath);
    CoTaskMemFree(filePath);
    return finalObject;
}