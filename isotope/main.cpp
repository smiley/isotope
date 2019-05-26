#include <Windows.h>

#include <Commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <assert.h>
#include <iostream>
#include <sstream>

#include "Config.h"
#include "Dialogues.h"
#include "ImageMaker.h"
#include "Shell.h"
#include "Types.h"
#include "Utilities.h"

inline bool isDriveSpecifier(const std::wstring& string) {
    if (string.length() < 2) {
        return false;
    }

    if ((towlower(string[0]) >= L'a') && (towlower(string[0]) <= L'z')) {
        // D...
        if (string[1] == L':') {
            // D:...
            if (string.length() == 2) {
                // D:
                return true;
            }

            if (string[2] == L'\\') {
                // D:\...
                if (string.length() == 3) {
                    // D:\ 
                    return true;
                } else {
                    // D:\Stuff...
                    return false;
                }
            }
        }
    }

    return false;
}

std::wstring GetSelectedDrive(const std::string& commandLine) {
    std::wstring selectedDrive(toWide(commandLine));

    if ((selectedDrive[0] == L'"') && (selectedDrive[selectedDrive.size() - 1] == L'"')) {
        selectedDrive = selectedDrive.substr(1, selectedDrive.size() - 2); // "-2" to skip the last quotation mark and
                                                                           // to account for the first quotation mark.
    }

    if (!selectedDrive.empty()) {
        if (!isDriveSpecifier(selectedDrive)) {
            MessageBoxA(nullptr, MESSAGES::WRONG_ARGUMENTS, APP_NAME, MB_ICONERROR);
            ExitProcess(1);
        }
    } else {
        auto availableDrives = GetDrivesOfType(DRIVE_CDROM);

        std::vector<std::wstring> usableDrives;
        for (const auto& drive : availableDrives) {
            if (isDiscInDrive(drive)) {
                usableDrives.push_back(drive);
            }
        }

        if (usableDrives.empty()) {
            if (availableDrives.size() > 0) {
                MessageBoxA(nullptr, MESSAGES::NO_DISCS, APP_NAME, MB_ICONERROR);
                ExitProcess(1);
            } else {
                MessageBoxA(nullptr, MESSAGES::NO_DRIVES, APP_NAME, MB_ICONERROR);
                ExitProcess(1);
            }
        }

        TASKDIALOGCONFIG taskDialog = { 0 };
        taskDialog.cbSize = sizeof(taskDialog);
        taskDialog.pszWindowTitle = APP_NAME;
        taskDialog.pszMainIcon = TD_INFORMATION_ICON;
        taskDialog.dwFlags = TDF_SIZE_TO_CONTENT | TDF_USE_COMMAND_LINKS;
        taskDialog.dwCommonButtons = TDCBF_CANCEL_BUTTON;
        taskDialog.pszMainInstruction = INSTRUCTIONS::PICK;
        taskDialog.pszContent = INSTRUCTIONS::PICK_EXPLAINED;

        TASKDIALOG_BUTTON driveSelections[26] = { 0 }; // A max of 26 logical drives can exist, as per the number of
                                                       // letters in the alphabet.

        for (unsigned int index = 0; index < usableDrives.size(); index++) {
            const std::wstring& drivePath = usableDrives[index];

            TASKDIALOG_BUTTON& current = driveSelections[index];
            current.nButtonID = 0x100 + index;

            std::wstring driveName(GetDriveName(drivePath));
            if (driveName.empty()) {
                driveName = MESSAGES::UNKNOWN;
            }

            std::wstringstream buttonText;
            buttonText << driveName << std::endl << SUBSTRINGS::AVAILABLE_AT << drivePath[0] << drivePath[1];

            // The string lives outside of this scope, so we use "wcsdup" here and "free" later. A string coming from
            // "std::basic_string" (any form of it) is safe for this function.
            wchar_t* buttonTextStr = _wcsdup(buttonText.str().c_str());
            current.pszButtonText = buttonTextStr;
        }

        taskDialog.cButtons = static_cast<UINT>(usableDrives.size());
        taskDialog.pButtons = driveSelections;

        taskDialog.nDefaultButton = IDCANCEL;

        int chosenButton = 0;
        TaskDialogIndirect(&taskDialog, &chosenButton, nullptr, nullptr);

        if (chosenButton == IDCANCEL) {
            // Okay to exit here with a "success" value, as the user picked "no" interactively.
            ExitProcess(0);
        } else {
            selectedDrive = usableDrives[static_cast<size_t>(chosenButton) - 0x100];
        }

        for (const auto& selection : driveSelections) {
            free(
                const_cast<void*>(
                    reinterpret_cast<const void*>(
                        selection.pszButtonText
                    )
                )
            );
        }
    }

    return selectedDrive;
}

HRESULT CALLBACK finishDialogCallback(HWND hwnd, UINT notification, WPARAM, LPARAM lparam, LONG_PTR path) {
    if (notification == TDN_HYPERLINK_CLICKED) {
        auto linkText = reinterpret_cast<wchar_t*>(lparam);
        auto outputPath = reinterpret_cast<std::wstring*>(path);
        if ((linkText == nullptr) || (outputPath == nullptr)) {
            // Abort!
            return S_FALSE;
        }

        if (_wcsicmp(linkText, OPEN) == 0) {
            try {
                OpenExplorerOnFile(*outputPath);
                return S_OK;
            } catch (const COMException&) {
                // Fall back to an indirect, process-executing approach instead.

                wchar_t explorerPath[MAX_PATH] = { 0 };
                unsigned int written = GetWindowsDirectory(explorerPath, _countof(explorerPath));
                if ((written > 0) && (written <= MAX_PATH)) {
                    if (PathAppend(explorerPath, FILES::EXPLORER_FILENAME) != FALSE) {
                        std::wstringstream commandLineBuf;
                        commandLineBuf << explorerPath << L" /select,\"" << *outputPath << L"\"";

                        std::wstring programPath(explorerPath);
                        std::wstring commandLine = commandLineBuf.str();
                        STARTUPINFO startup = { 0 };
                        PROCESS_INFORMATION process = { 0 };

                        if (CreateProcess(nullptr, const_cast<wchar_t*>(commandLine.c_str()),
                                          nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process)
                            != FALSE) {
                            CloseHandle(process.hProcess);
                            CloseHandle(process.hThread);

                            return S_OK;
                        }
                    }
                }
            }
            TaskDialog(hwnd, nullptr, APP_NAME, MESSAGES::OPEN_FAILED_TITLE,
                       MESSAGES::OPEN_FAILED_FULL, 0, TD_ERROR_ICON, nullptr);
        }
    }

    return S_OK;
}

void copyDialogFinishHandler(ProgressDialog& instance, void* context) {
    auto outputPath = reinterpret_cast<std::wstring*>(context);
    instance.stop();

    std::wstringstream msg;
    msg << MESSAGES::RIP_SUCCESS << L" <A HREF=\"open\">" << *outputPath << L"</A>";
    std::wstring msgString = msg.str();

    TASKDIALOGCONFIG taskDialog = { 0 };
    taskDialog.cbSize = sizeof(taskDialog);
    taskDialog.pszWindowTitle = APP_NAME;
    taskDialog.pszMainIcon = TD_INFORMATION_ICON;
    taskDialog.dwFlags = TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS;
    taskDialog.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    taskDialog.pszMainInstruction = MESSAGES::COPY_COMPLETE;
    taskDialog.pszContent = msgString.c_str();

    taskDialog.lpCallbackData = reinterpret_cast<LONG_PTR>(context);
    taskDialog.pfCallback = finishDialogCallback;

    TaskDialogIndirect(&taskDialog, nullptr, nullptr, nullptr);
}


int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
    std::wstring selectedDrive = GetSelectedDrive(lpCmdLine);

    SaveFileDialog outputSelect;
    outputSelect.setDefaultExtension(FILES::IMG);
    outputSelect.setFileTypes({ { FILES::IMG_DESCRIPTION, FILES::IMG_FILTER } });
    std::wstring outputPath = outputSelect.pick();
    if (outputPath.empty()) {
        // Cancelled. Goodbye!
        return 0;
    }

    ProgressDialog dialog(std::wstring(APP_NAME.wide()));

    dialog.setCompletionHandler(copyDialogFinishHandler, &outputPath);

    dialog.setDisplayText(MESSAGES::COPY_DIALOG_TITLE, SUBSTRINGS::COPY_DIALOG_DESTINATION + outputPath);

    ImageMaker maker(selectedDrive, outputPath);
    dialog.setCancelHandler([] (ProgressDialog& instance, void* context) {
        auto maker = reinterpret_cast<ImageMaker*>(context);

        maker->stop();
        instance.stop();
    }, &maker);

    maker.setProgressHandler([] (uint64_t bytesCopied, uint64_t bytesTotal, void* context) {
        auto dialog = reinterpret_cast<ProgressDialog*>(context);

        dialog->setProgress(bytesCopied, bytesTotal);
    }, &dialog);

    dialog.start(ProgressDialog::Options::EstimateTime);
    maker.start();
    dialog.stop();

    return 0;
}