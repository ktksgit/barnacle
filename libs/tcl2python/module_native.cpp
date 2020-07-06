#include "embed.h"

#include <iostream>
#include <thread>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <Tlhelp32.h>
#include <winternl.h>

namespace py = pybind11;

namespace
{

static const constexpr auto ANTI_DEBUG_THREAD_ENTRY = 0xf15cfd;

std::string getErrorAsStr(DWORD error)
{
    LPSTR msg_buffer;
    auto buffer_length = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        reinterpret_cast<LPSTR>(&msg_buffer),
        0, nullptr);
    if (buffer_length)
    {
        std::string result{msg_buffer, msg_buffer + buffer_length};

        LocalFree(msg_buffer);

        return result;
    }

    return {};
}

void killAntiDebugThread() {
    DWORD process_id = GetCurrentProcessId();
    auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, process_id);

    if(snapshot_handle == INVALID_HANDLE_VALUE) {
        return;
    }

    auto thread_entry = THREADENTRY32{};
    thread_entry.dwSize = sizeof(THREADENTRY32);
    auto success = Thread32First(snapshot_handle, &thread_entry);
    if (!success) {
        auto err = GetLastError();
        if (err == ERROR_NO_MORE_FILES) {
            throw std::runtime_error(getErrorAsStr(err));
        }
    }

    std::vector<DWORD> threads;
    while (true) {
        if (thread_entry.th32OwnerProcessID == process_id) {
            auto thread_id = thread_entry.th32ThreadID;
            threads.push_back(thread_id);
        }

        auto success = Thread32Next(snapshot_handle, &thread_entry);
        if (!success) {
            auto err = GetLastError();
            if (err == ERROR_NO_MORE_FILES) {
                break;
            }

            std::cout << getErrorAsStr(err) << std::endl;
        }
    }
    
    CloseHandle(snapshot_handle);

    for(auto thread_id : threads) {
        HANDLE thread_handle = OpenThread(THREAD_ALL_ACCESS, false, thread_id);
        PVOID thread_info;
        ULONG return_length;
        NtQueryInformationThread(thread_handle, ThreadQuerySetWin32StartAddress, &thread_info, sizeof(thread_info), &return_length);

        if (reinterpret_cast<int>(thread_info) == ANTI_DEBUG_THREAD_ENTRY) {
            std::cout << "Killing antidebug thread " << std::endl;
            TerminateThread(thread_handle, 0);
        }
        CloseHandle(thread_handle);
    }
}

void waitForDebuggerAndBreak() {
    while(!IsDebuggerPresent()) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
    __asm__("int $3");
}


} // namespace

PYBIND11_EMBEDDED_MODULE(_native, m) {
    m.def("killAntiDebugThread", killAntiDebugThread);
    m.def("waitForDebuggerAndBreak", waitForDebuggerAndBreak);
}
