#include "Types.h"
#include <sstream>
#include <assert.h>

inline BOOL toBOOL(bool value) {
    if (value) {
        return TRUE;
    }
    
    return FALSE;
}

HANDLE duplicateHandle(HANDLE other) {
    HANDLE duplicated = INVALID_HANDLE_VALUE;

    BOOL success = DuplicateHandle(GetCurrentProcess(), other,
                                   GetCurrentProcess(), &duplicated, 0, FALSE, DUPLICATE_SAME_ACCESS);

    if (success == FALSE) {
        throw WinAPIException(GetLastError());
    }

    return duplicated;
}

Handle::Handle(HANDLE otherHandle) : m_handle(otherHandle) {
    if (otherHandle == INVALID_HANDLE_VALUE) {
        throw WinAPIException(GetLastError());
    }
};

Handle::~Handle() {
    CloseHandle(m_handle);
}

HANDLE Handle::getHandle() const {
    return m_handle;
}

Event::Event(bool manualReset, bool initialState) : 
    m_eventHandle(CreateEvent(nullptr, toBOOL(manualReset), toBOOL(initialState), nullptr)) {}

Event::Event(Event& other) :
    m_eventHandle(duplicateHandle(other.m_eventHandle.getHandle())) {}

bool Event::isSignaled() {
    auto res = WaitForSingleObject(m_eventHandle.getHandle(), 0);
    switch (res) {
    case WAIT_OBJECT_0:
        return true;
    case WAIT_TIMEOUT:
        return false;
    default:
        throw WinAPIException(GetLastError());
    }
}

void Event::signal() {
    if (SetEvent(m_eventHandle.getHandle()) == FALSE) {
        throw WinAPIException(GetLastError());
    }
}

HANDLE Event::getHandle() {
    return m_eventHandle.getHandle();
}

std::string SystemException::composeErrorString(HRESULT err) {
    char* errorString = nullptr;

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&errorString),
                   0, nullptr);

    if (errorString != nullptr) {
        auto ret = std::string(errorString);

        LocalFree(errorString);
        return ret;
    }

    return std::string();
}
