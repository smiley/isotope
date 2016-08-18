#include "Dialogues.h"
#include "Types.h"

#include <process.h>
#include <versionhelpers.h>

#include "Shell.h"

const char* ASSERT_INSTANCE_ERROR_STRING = "Not bound to a real COM object!";

// Implemented as a macro to ease debugging when "always_assert" quotes the filename & line.
#define assert_instance() always_assert(m_instance != nullptr, ASSERT_INSTANCE_ERROR_STRING)

const std::wstring ProgressDialog::UNSPECIFIED = std::wstring();

#pragma region ProgressDialog

ProgressDialog::ProgressDialog(const std::wstring& title, HWND parent) :
    m_parent(parent) {

    CLSID cls = { 0 };
    HRESULT status = CLSIDFromString(CLASS_GUID, &cls);
    if (FAILED(status)) {
        throw std::runtime_error("Cannot find COM class.");
    }

    status = CoCreateInstance(cls, nullptr, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (void**)&m_instance);
    if (FAILED(status)) {
        throw std::runtime_error("Cannot create IProgressDialog object!");
    }

    setTitle(title);
}

ProgressDialog::~ProgressDialog() {
    if (WaitForSingleObject(m_cancelThread, INFINITE) == WAIT_FAILED) {
        // D:
        TerminateThread(m_cancelThread, 0xFFFFFFFF);
    }

    auto localPointer = m_instance;
    m_instance = nullptr;
    localPointer->Release();
}

void ProgressDialog::setTitle(const std::wstring & title) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetTitle(title.c_str()));
}

void ProgressDialog::setCancelText(const std::wstring & text) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetCancelMsg(text.c_str(), nullptr));
}

void ProgressDialog::setDisplayText(const std::wstring& lineOne,
                                    const std::wstring& lineTwo,
                                    const std::wstring& lineThree) {
    assert_instance();

    // The developer might want to clear a line, and specifying an empty string here is 
    // allowed. BUT: we want the sematic option of *leaving out* a parameter, to mark us not
    // to modify that line. But C++ references cannot be optional, thus we use a static
    // instance to remedy this situation.
    if ((&lineOne == &UNSPECIFIED) && (&lineTwo == &UNSPECIFIED) && (&lineThree == &UNSPECIFIED)) {
        throw std::logic_error("You must specify at least one line to change.");
    }

    if (&lineOne != &UNSPECIFIED) {
        setLine(1, lineOne);
    }
    if (&lineTwo != &UNSPECIFIED) {
        setLine(2, lineTwo);
    }
    if (&lineThree != &UNSPECIFIED) {
        setLine(3, lineThree);
    }
}

void ProgressDialog::setProgress(uint64_t currentAmount, uint64_t totalAmount) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetProgress64(currentAmount, totalAmount));
    m_progress = currentAmount;
    m_total = totalAmount;

    if (hasCompleted()) {
        // Fetch it locally to avoid a race condition here.
        EventHandler completionHandler = m_completionHandler;

        if (completionHandler != nullptr) {
            completionHandler(*this, m_completionHandlerContext);
        }

        setCompletionHandler(nullptr);
    }
}

uint64_t ProgressDialog::addToProgress(uint64_t addition) {
    assert_instance();

    setProgress(m_progress + addition, m_total);

    return m_progress;
}

void ProgressDialog::setTotal(uint64_t total) {
    setProgress(m_progress, total);
}

bool ProgressDialog::hasCancelled() {
    assert_instance();

    return m_instance->HasUserCancelled() != FALSE;
}

bool ProgressDialog::hasCompleted() {
    return m_progress >= m_total;
}

void ProgressDialog::setCancelHandler(EventHandler function, void* context) {
    InterlockedExchange(reinterpret_cast<volatile DWORD_PTR*>(&m_cancelHandler),
                        reinterpret_cast<DWORD_PTR>(function));
    InterlockedExchange(reinterpret_cast<volatile DWORD_PTR*>(&m_cancelHandlerContext),
                        reinterpret_cast<DWORD_PTR>(context));
}

void ProgressDialog::setCompletionHandler(EventHandler function, void* context) {
    InterlockedExchange(reinterpret_cast<volatile DWORD_PTR*>(&m_completionHandler),
                        reinterpret_cast<DWORD_PTR>(function));
    InterlockedExchange(reinterpret_cast<volatile DWORD_PTR*>(&m_completionHandlerContext),
                        reinterpret_cast<DWORD_PTR>(context));
}

uint64_t ProgressDialog::getProgress() {
    return m_progress;
}

uint64_t ProgressDialog::getTotal() {
    return m_total;
}

void ProgressDialog::start(Options options) {
    assert_instance();

    DWORD realOptions = static_cast<DWORD>(options);

    if (realOptions & static_cast<DWORD>(Options::Indeterminate)) {
        realOptions -= static_cast<DWORD>(Options::Indeterminate);
        if (IsWindowsVistaOrGreater()) {
            realOptions |= PROGDLG_MARQUEEPROGRESS;
        } else {
            realOptions |= PROGDLG_NOPROGRESSBAR;
        }
    }

    THROW_IF_FAILED(m_instance->StartProgressDialog(m_parent, nullptr, realOptions, nullptr));
    m_cancelThread = reinterpret_cast<HANDLE>(_beginthread(hasCancelledThreadFunction, 0, this));
}

void ProgressDialog::stop() {
    assert_instance();

    THROW_IF_FAILED(m_instance->StopProgressDialog());
    m_cancelThreadShouldStop.signal();
}

void ProgressDialog::CloseHandler(ProgressDialog& instance, void*) {
    instance.stop();
}

void ProgressDialog::hasCancelledThreadFunction(void* instance) {
    always_assert(instance != nullptr, "Given instance is NULL!");

    auto that = reinterpret_cast<ProgressDialog*>(instance);
    that->hasCancelledProcedure();
}

void ProgressDialog::hasCancelledProcedure() {
    do {
        if (hasCancelled()) {
            // Fetch it locally to avoid a race condition here.
            EventHandler cancelHandler = m_cancelHandler;

            if (cancelHandler != nullptr) {
                cancelHandler(*this, m_cancelHandlerContext);
            }

            setCancelHandler(nullptr);
            break;
        }

        Sleep(3000);
    } while (!m_cancelThreadShouldStop.isSignaled());
}

void ProgressDialog::setLine(unsigned int lineNumber, const std::wstring & line) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetLine(lineNumber, line.c_str(), TRUE, nullptr));
}

ProgressDialog::Options operator|(ProgressDialog::Options one,
                                  ProgressDialog::Options other) {
    return static_cast<ProgressDialog::Options>(static_cast<DWORD>(one) |
                                                static_cast<DWORD>(other));
}

#pragma endregion

#pragma region FileDialog

FileDialog::FileDialog() {
    CoInitialize(nullptr);
}

FileDialog::~FileDialog() {
    auto localPointer = m_instance;
    m_instance = nullptr;
    localPointer->Release();
    CoUninitialize();
}

OpenFileDialog::OpenFileDialog() {
    HRESULT status = CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                                      CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (void**)&m_instance);

    if (FAILED(status)) {
        throw std::runtime_error("Cannot create IFileOpenDialog object!");
    }
}

SaveFileDialog::SaveFileDialog() {
    HRESULT status = CoCreateInstance(CLSID_FileSaveDialog, nullptr,
                                      CLSCTX_INPROC_SERVER, IID_IFileSaveDialog, (void**)&m_instance);

    if (FAILED(status)) {
        throw std::runtime_error("Cannot create IFileSaveDialog object!");
    }
}

std::wstring FileDialog::getFileName() {
    assert_instance();

    wchar_t* fileName = nullptr;

    THROW_IF_FAILED(m_instance->GetFileName(&fileName));

    std::wstring finalObject(fileName);
    CoTaskMemFree(fileName);
    return finalObject;
}

std::wstring FileDialog::getPath() {
    assert_instance();

    IShellItem* result = nullptr;

    THROW_IF_FAILED(m_instance->GetResult(&result));

    auto resolved = ResolveShellItem(result);
    result->Release();
    return resolved;
}

void FileDialog::setFileName(const std::wstring& filename) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetFileName(filename.c_str()));
}

void FileDialog::setFileTypes(const std::vector<COMDLG_FILTERSPEC>& fileTypes) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetFileTypes(static_cast<UINT>(fileTypes.size()), fileTypes.data()));
}

void FileDialog::setDefaultExtension(const std::wstring& extension) {
    assert_instance();

    if (extension[0] == L'.') {
        if (extension.size() == 1) {
            // About to go off-bounds here. Abort.
            throw std::logic_error("Empty extension given.");
        }
        THROW_IF_FAILED(m_instance->SetDefaultExtension(extension.substr(1).c_str()));
    } else {
        THROW_IF_FAILED(m_instance->SetDefaultExtension(extension.c_str()));
    }    
}

void FileDialog::setTitle(const std::wstring& title) {
    assert_instance();

    THROW_IF_FAILED(m_instance->SetTitle(title.c_str()));
}

std::wstring FileDialog::pick() {
    assert_instance();

    HRESULT status = m_instance->Show(nullptr);
    if (status == S_OK) {
        return getPath();
    } else if (status == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        return std::wstring();
    } else {
        throw COMException(status);
    }
}

#pragma endregion
