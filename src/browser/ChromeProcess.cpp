#include "browser/ChromeProcess.h"
#include <chrono>
#include <filesystem>

ChromeProcess::ChromeProcess()
    : hJob_(nullptr)
{
    chromePath_ =R"(C:\Users\garvc\Desktop\CQ\chrome\win64-150.0.7871.46\chrome-win64\chrome.exe)";
    auto id = std::chrono::steady_clock::now().time_since_epoch().count();
    profilePath_ =(std::filesystem::temp_directory_path() /("crawler_profile_" + std::to_string(id))).string();
    std::filesystem::create_directories(profilePath_);
}

ChromeProcess::~ChromeProcess()
{
    stop();
    try
    {
        std::filesystem::remove_all(profilePath_);
    }
    catch (...)
    {
    }
}

std::string ChromeProcess::buildCommand(bool headless) const
{
    std::string command;
    command += "\"" + chromePath_ + "\" ";
    command += "--remote-debugging-port=9222 ";
    command += "--user-data-dir=\"" + profilePath_ + "\" ";
    if (headless)
    {
        command += "--headless=new ";
        command += "--disable-gpu ";
    }
    command += "--no-first-run ";
    command += "--no-default-browser-check ";
    command += "--disable-extensions ";
    command += "about:blank";
    return command;
}

bool ChromeProcess::start(bool headless)
{
    if (running_)return true;

    hJob_ = CreateJobObjectA(nullptr, nullptr);
    if (hJob_)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {}; //initialize to 0
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE; //when jobobjectisdestoryed automatically terminate this too
        SetInformationJobObject(hJob_, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
    }

    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    if (headless)
    {
        startup.dwFlags = STARTF_USESHOWWINDOW;
        startup.wShowWindow = SW_HIDE;
    }

    std::string command = buildCommand(headless);
    BOOL success =
        CreateProcessA(
            nullptr,//app name
            command.data(),//command
            nullptr,//default process sec
            nullptr,//default thread sec
            FALSE,//dont inherit handles from current process
            0,//no special creation flags
            nullptr,//env
            nullptr,//working dir
            &startup,//startup settings
            &processInfo_);//windows fill this - process handle , thread handle , ID 
    if (!success)
    {
        if (hJob_)
        {
            CloseHandle(hJob_);
            hJob_ = nullptr;
        }
        return false;
    }

    if (hJob_)
    {
        AssignProcessToJobObject(hJob_, processInfo_.hProcess);//assign chrome to hjob
    }

    running_ = true;    
    Sleep(1000);
    return true;
}

void ChromeProcess::stop()
{
    if (!running_)return;
    TerminateProcess(processInfo_.hProcess, 0);
    CloseHandle(processInfo_.hThread);
    CloseHandle(processInfo_.hProcess);
    if (hJob_)
    {
        CloseHandle(hJob_);
        hJob_ = nullptr;
    }
    running_ = false;
}

bool ChromeProcess::isRunning() const
{
    return running_;
}