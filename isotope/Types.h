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

class Buffer {
    class Pointer {
        Pointer(Buffer& container) : m_container(container) {};
    public:
        template<typename PointerType>
        operator PointerType*() {
            return reinterpret_cast<PointerType*>(m_container.m_pointer);
        }

        template<typename PointerType>
        operator const PointerType*() const {
            return reinterpret_cast<const PointerType*>(m_container.m_pointer);
        }

    private:
        Buffer& m_container;

        // Disallow copy/assignment/move.
        Pointer(const Pointer&);
        Pointer(Pointer&&);
        Pointer& operator=(const Pointer&);

        // "Buffer" needs access to its inner class.
        friend Buffer;
    };

public:
    // "calloc" also zero-initializes the memory block it allocates.
    Buffer(size_t size) : size(size), m_pointer(calloc(1, size)), address(*this) {
        if (m_pointer == nullptr) {
            throw std::bad_alloc();
        }
    };

    template<typename StringType>
    Buffer(const std::basic_string<StringType> source) :
        size(source.size() * sizeof(StringType)),
        m_pointer(calloc(1, size)),
        address(*this) {
        
        if (m_pointer == nullptr) {
            throw std::bad_alloc();
        }

        memcpy(m_pointer, source.data(), size);
    }

    ~Buffer() {
        free(m_pointer);
    }

    const size_t size;
    Pointer address;

private:
    void* m_pointer;
};