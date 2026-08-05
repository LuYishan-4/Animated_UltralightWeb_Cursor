#pragma once

#include <iostream>
#include <fstream>
#include <csignal>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <string>

namespace UltralightWebCursorM {

class CrashHandler {
public:
    static void registerHandler() {
        std::signal(SIGSEGV, handleCrash);
        std::signal(SIGFPE,  handleCrash); 
        std::signal(SIGABRT, handleCrash); 
        std::signal(SIGILL,  handleCrash); 
    }

private:
    static void handleCrash(int signalNumber) {
        std::time_t now = std::time(nullptr);
        char timeStr[20];
        std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", std::localtime(&now));
        std::string filename = "/tmp/kwin_effect_crash_" + std::string(timeStr) + "_pid" + std::to_string(getpid()) + ".log";
        std::ofstream logFile(filename);
        if (!logFile.is_open()) {
            std::exit(signalNumber);
        }

        logFile << "========================================\n";
        logFile << "        Crash     \n";
        logFile << "========================================\n";
        logFile << "code " << signalNumber << "\n";
        logFile << "PID: " << getpid() << "\n";
        logFile << "time: " << timeStr << "\n\n";
        logFile << "(Stack Trace)]:\n";

        void* array[50];
        int size = backtrace(array, 50);
        char** symbols = backtrace_symbols(array, size);

        if (symbols != nullptr) {
            for (int i = 0; i < size; ++i) {
                logFile << "[" << i << "] " << symbols[i] << "\n";
            }
            std::free(symbols);
        }

        logFile << "========================================\n";
        logFile.close();
        std::signal(signalNumber, SIG_DFL);
        std::raise(signalNumber);
    }
};

} // namespace KWin
