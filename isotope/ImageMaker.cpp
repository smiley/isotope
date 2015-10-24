#include "ImageMaker.h"

#include <memory>
#include <process.h>

#include "Types.h"
#include "Utilities.h"

const std::wstring RAW_VOLUME_PREFIX = std::wstring(L"\\\\.\\");

const auto CHUNK_SIZE = 5 * 1024 * 1024;

ImageMaker::ImageMaker(const std::wstring& driveRoot, const std::wstring & outputPath) :
    m_drive(driveRoot), m_output(outputPath) {
    if (driveRoot.size() > 3) {
        throw std::logic_error("You must specify the root of a drive, not a full path to a file.");
    } else if (driveRoot.size() < 2) {
        throw std::logic_error("You must specify more than just a drive letter.");
    }

    ULARGE_INTEGER totalSize = { 0 };

    if (SHGetDiskFreeSpace(m_drive.c_str(), nullptr, &totalSize, nullptr) == FALSE) {
        throw WinAPIException(GetLastError());
    }

    m_totalSize = totalSize.QuadPart;

    std::wstring rawVolumeRoot = RAW_VOLUME_PREFIX + driveRoot.substr(0, 2);

    m_driveHandle = CreateFile(rawVolumeRoot.c_str(), GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                              FILE_FLAG_SEQUENTIAL_SCAN, INVALID_HANDLE_VALUE);

    if (m_driveHandle == INVALID_HANDLE_VALUE) {
        throw WinAPIException(GetLastError());
    }
};

ImageMaker::~ImageMaker() {
    stop();
    CloseHandle(m_driveHandle);
}

void ImageMaker::setProgressHandler(ProgressHandler handler, void* context) {
    InterlockedExchange(reinterpret_cast<volatile DWORD_PTR*>(&m_handler),
                        reinterpret_cast<DWORD_PTR>(handler));
    InterlockedExchange(reinterpret_cast<volatile DWORD_PTR*>(&m_handlerContext),
                        reinterpret_cast<DWORD_PTR>(context));
}

void ImageMaker::start() {
    uint64_t bytesCopied = 0;

    std::unique_ptr<void, FreeDeleter> buffer(malloc(CHUNK_SIZE));

    Handle outputHandle(CreateFile(m_output.c_str(), GENERIC_WRITE, 0, nullptr,
                                   CREATE_ALWAYS, 0, INVALID_HANDLE_VALUE));

    do {
        DWORD bytesRead = 0;
        if (ReadFile(m_driveHandle, buffer.get(), CHUNK_SIZE, &bytesRead, nullptr) == FALSE) {
            if (GetLastError() != ERROR_IO_PENDING) {
                if (GetLastError() == ERROR_OPERATION_ABORTED) {
                    // Cancelled!
                    break;
                }
                throw WinAPIException(GetLastError());
            }
        }

        DWORD bytesWritten = 0;
        if (WriteFile(outputHandle.getHandle(), buffer.get(), bytesRead, &bytesWritten, nullptr) == FALSE) {
            if (GetLastError() != ERROR_IO_PENDING) {
                if (GetLastError() == ERROR_OPERATION_ABORTED) {
                    // Cancelled!
                    break;
                }
                throw WinAPIException(GetLastError());
            }
        }
        always_assert(bytesRead == bytesWritten, "Mismatch between read & write! No space left?");

        bytesCopied += bytesRead;

        ProgressHandler handler = m_handler;
        if (handler != nullptr) {
            handler(bytesCopied, m_totalSize, m_handlerContext);
        }
    } while ((!m_shouldHalt.isSignaled()) && (bytesCopied < m_totalSize));
}

HANDLE ImageMaker::startAsync() {
    HANDLE threadHandle = reinterpret_cast<HANDLE>(_beginthread([] (void* that) {
        auto instance = reinterpret_cast<ImageMaker*>(that);
        instance->start();
    }, 0, this));

    if (threadHandle == reinterpret_cast<HANDLE>(-1)) {
        throw WinAPIException(GetLastError());
    }

    return threadHandle;
}

void ImageMaker::stop() {
    CancelIoEx(m_driveHandle, nullptr);
    m_shouldHalt.signal();
}
