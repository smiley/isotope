#include "Shell.h"
#include "Types.h"

#include <Windows.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#include <memory>

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

IShellItem* CreateShellItemFromPath(const std::wstring& path) {
    IShellItem* item = nullptr;
    THROW_IF_FAILED(SHCreateItemFromParsingName(path.c_str(), nullptr,
                                                IID_PPV_ARGS(&item)));
    return item;
}

void OpenExplorerOnFile(const std::wstring& path) {
    LPSHELLFOLDER shellFolderInterface = nullptr;
    PIDLIST_RELATIVE file = nullptr;

    THROW_IF_FAILED(SHGetDesktopFolder(&shellFolderInterface));

    THROW_IF_FAILED(shellFolderInterface->ParseDisplayName(nullptr,
                                                           nullptr,
                                                           const_cast<wchar_t*>(path.c_str()),
                                                           // Documentation states this param will
                                                           // not be changed.
                                                           nullptr,
                                                           &file,
                                                           nullptr));

    // Calling "SHOpenFolderAndSelectItems" with a single item opens its parent folder and selects it.
    THROW_IF_FAILED(SHOpenFolderAndSelectItems(file, 0, nullptr , 0));
}

std::wstring Path::GetParentFolder(const std::wstring& path) {
    // Path functions change their input, so copy the input to a buffer.
    // ("std::basic_string" objects cannot be changed)
    Buffer temporary(path);

    // No matter the outcome, we return what was returned to us. In case input was bad (no folders
    // above) we can still return the same input, as path functions do.
    PathRemoveFileSpec(temporary.address);

    return std::wstring(temporary.address);
}
