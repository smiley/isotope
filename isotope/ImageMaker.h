#pragma once

#include "Windows.h"

#include <string>

#include "Types.h"

class ImageMaker {
public:
    using ProgressHandler = void(*)(uint64_t bytesRead, uint64_t totalBytes, void* context);

    ImageMaker(const std::wstring& drivePath, const std::wstring& outputPath);
    ~ImageMaker();

    void setProgressHandler(ProgressHandler handler, void* context = nullptr);

    void start();
    HANDLE startAsync();
    void stop();

private:
    std::wstring m_drive;
    std::wstring m_output;

    uint64_t m_totalSize = 0;

    HANDLE m_driveHandle = INVALID_HANDLE_VALUE;
    Event m_shouldHalt;

    ProgressHandler m_handler = nullptr;
    void* m_handlerContext = nullptr;
};