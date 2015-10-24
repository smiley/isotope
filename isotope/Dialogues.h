#pragma once

#include <windows.h>
#include <Shlobj.h>

#include <string>
#include <vector>

#include "Types.h"

class ProgressDialog {
public:
    using EventHandler = void(*)(ProgressDialog& instance, void* context);
    enum DialogOptions {
        Normal = PROGDLG_NORMAL,
        // The progress dialog box will be modal to the window specified by hwndParent. By default, a progress
        // dialog box is modeless.
        Modal = PROGDLG_MODAL,
        // Automatically estimate the remaining time and display the estimate on line 3. If this is used, you can only
        // set lines 1 & 2 in "setDisplayText".
        EstimateTime = PROGDLG_AUTOTIME,
        // Do not show the "time remaining" text.
        NoTimeEstimation = PROGDLG_NOTIME,
        // Do not display a minimize button on the dialog box's caption bar.
        NoMinimize = PROGDLG_NOMINIMIZE,
        // Do not display a progress bar. Use when you cannot estimate progress for the current operation.
        //
        // Not recommended! Use "Indeterminate" instead, which uses an indeterminate progress bar on Vista and above,
        // while falling back to no progress bar on XP and below.
        NoProgressBar = PROGDLG_NOPROGRESSBAR,
        // Use this when you wish to indicate that progress is being made, but the time
        // required for the operation is unknown.
        Indeterminate = 0x00000100,
        // Use this when you wish to indicate that progress is being made, but the time
        // required for the operation is unknown.
        //
        // Supported from Vista and up. Use "Indeterminate" to use this on Vista and newer, while making use
        // of "NoProgressBar" as a fallback on XP.
        MarqueeProgressBar = PROGDLG_MARQUEEPROGRESS,
        // Do not display a cancel button. The operation cannot be canceled. Use this only when absolutely necessary.
        NonCancellable = PROGDLG_NOCANCEL
    };

    ProgressDialog(const std::wstring& title = UNSPECIFIED, HWND parent = nullptr);
    ~ProgressDialog();

    void setTitle(const std::wstring& title);
    void setCancelText(const std::wstring& text);
    void setDisplayText(const std::wstring& lineOne = UNSPECIFIED,
                        const std::wstring& lineTwo = UNSPECIFIED,
                        const std::wstring& lineThree = UNSPECIFIED);

    void setProgress(uint64_t currentAmount, uint64_t totalAmount);
    uint64_t addToProgress(uint64_t addition);
    void setTotal(uint64_t total);

    // Synchronous-use methods.
    bool hasCancelled();
    bool hasCompleted();

    // Async-use methods.
    void setCancelHandler(EventHandler function, void* context = nullptr);
    void setCompletionHandler(EventHandler function, void* context = nullptr);

    uint64_t getProgress();
    uint64_t getTotal();

    void start(DialogOptions options = Normal);
    void start(DWORD /* OR-ed DialogOptions */ options);
    void stop();

    // Default handler for "setCancelHandler" & "setCompletionHandler" for apps that only want to close the dialog when
    // the user calls "cancel" or the operation ends.
    static void CloseHandler(ProgressDialog& instance, void* context);

private:
    static void __cdecl hasCancelledThreadFunction(void* instance);
    void hasCancelledProcedure();
    void setLine(unsigned int lineNumber, const std::wstring& line);

    IProgressDialog* m_instance = nullptr;

    uint64_t m_progress = 0;
    uint64_t m_total = 0;

    DWORD m_flags = PROGDLG_NORMAL;
    HWND m_parent = nullptr;

    EventHandler m_cancelHandler = nullptr;
    void* m_cancelHandlerContext = nullptr;
    EventHandler m_completionHandler = nullptr;
    void* m_completionHandlerContext = nullptr;

    Event m_cancelThreadShouldStop;
    HANDLE m_cancelThread = INVALID_HANDLE_VALUE;

    static const std::wstring UNSPECIFIED;
    const wchar_t* CLASS_GUID = L"{F8383852-FCD3-11d1-A6B9-006097DF5BD4}";
};

// Mark "NoProgressBar" as deprecated.
#pragma deprecated(NoProgressBar)

class FileDialog {
public:
    FileDialog();
    ~FileDialog();

    std::wstring getFileName();
    std::wstring getPath();

    void setFileName(const std::wstring& filename);
    void setFileTypes(const std::vector<COMDLG_FILTERSPEC>& fileTypes);
    void setDefaultExtension(const std::wstring& extension);
    void setTitle(const std::wstring& title);

    std::wstring pick();

protected:
    IFileDialog* m_instance = nullptr;
};

class OpenFileDialog : public FileDialog {
public:
    OpenFileDialog();
    ~OpenFileDialog() {}; // ~FileDialog handles destruction.
};

class SaveFileDialog : public FileDialog {
public:
    SaveFileDialog();
    ~SaveFileDialog() {}; // ~FileDialog handles destruction.
};

