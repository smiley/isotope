#pragma once

#include "Types.h"

#define APP_NAME_STR "Isotope"

class {
public:
    const char* APP_NAME_A = APP_NAME_STR;
    const wchar_t* APP_NAME_W = COMBINE(L, APP_NAME_STR);

    operator const char*() {
        return APP_NAME_A;
    }

    operator const wchar_t*() {
        return APP_NAME_W;
    }

    const char* narrow() {
        return *this;
    }

    const wchar_t* wide() {
        return *this;
    }
} APP_NAME;

// Messages
namespace MESSAGES {
    // Errors
    const auto* WRONG_ARGUMENTS = "Wrong arguments given. The correct format is:\n"  \
                                  "isotope.exe <disc drive>\n\n"                     \
                                                                                     \
                                  "Or with no arguments to pick one interactively.";

    const auto* NO_DISCS = "You have several drives, but none contain a disc.";
    const auto* NO_DRIVES = "You have no disc drives. Why are you even running this?";
    const auto* UNKNOWN = L"Unknown";
    const auto* OPEN_FAILED_TITLE = L"Open failed";
    const auto* OPEN_FAILED_FULL = L"We tried to open the destination, but " \
                                   L"we ultimately failed. Sorry about that.";

    // Successes
    const auto* RIP_SUCCESS = L"The disc was successfully ripped to:";

    const auto* COPY_COMPLETE = L"Copy complete!";
    const auto* COPY_DIALOG_TITLE = L"Copying disc to image file";
}

// Instructions
namespace INSTRUCTIONS {
    const auto* PICK = L"Pick a disc to rip into an ISO image";
    const auto* PICK_EXPLAINED = \
        L"The following drives have been recognized on your system. If any disc drives are " \
        L"missing, they are unusable due to one of many reasons: there's currently no disc " \
        L"inserted, the disc type is unrecognized, or something is wrong with the drive.";
}

// Substrings
namespace SUBSTRINGS {
    const auto* AVAILABLE_AT = L"Available at ";
    const auto COPY_DIALOG_DESTINATION = std::wstring(L"Destination: ");
}

// File chooser strings.
namespace FILES {
    const auto* IMG = L"img";
    const auto* IMG_FILTER = L"*.img";
    const auto* IMG_DESCRIPTION = L"Image files";

    const auto* EXPLORER_FILENAME = L"explorer.exe";
}

// No category (or need for)
const auto* OPEN = L"open";