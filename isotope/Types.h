#pragma once

#include <Windows.h>
#include <string>

#define always_assert(val, msg) \
    if (!(val)) { \
        throw AssertionException(msg); \
    }

#define _COMBINE(X,Y) X##Y          // helper macro
#define COMBINE(X,Y) _COMBINE(X,Y)

#define THROW_IF_FAILED(action) HRESULT COMBINE(status, __LINE__) = action; \
    if FAILED(COMBINE(status, __LINE__)) { \
        throw COMException(COMBINE(status, __LINE__)); \
    }

struct RunArguments {
    std::wstring selectedDrive;
    std::wstring outputPath;
};

class SystemException : public std::runtime_error {
public:
    SystemException(const std::string& name, HRESULT err) :
        std::runtime_error(composeErrorString(err)) {
        OutputDebugStringA((name + std::string(" error: ") + what()).c_str());
    };

private:
    static std::string composeErrorString(HRESULT err);
};

class COMException : public SystemException {
public:
    COMException(HRESULT err) : SystemException("COM", err) {};
};

class WinAPIException : public SystemException {
public:
    WinAPIException(DWORD err) : SystemException("WinAPI", err) {};
};

class AssertionException : public std::runtime_error {
public:
    AssertionException(const char* msg) :
        std::runtime_error(msg) {};

    AssertionException(const std::string& msg) :
        AssertionException(msg.c_str()) {};
};

class Handle {
public:
    explicit Handle(HANDLE otherHandle);
    ~Handle();

    HANDLE getHandle() const;

private:
    HANDLE m_handle;
};

class Event {
public:
    Event(bool manualReset=true, bool initialState=false);
    Event(Event& other);
    ~Event() {};

    bool isSignaled();
    void signal();
    HANDLE getHandle();

private:
    Handle m_eventHandle;
};