//#include "threadfuncs.h"
#include "threadfuncs.h"
#include <stdexcept>

int normalCounter = 0;
std::atomic<int> atomicCounter{0};

Logger::Logger(const std::string& filename)
    : file_(filename, std::ios::out | std::ios::trunc) {
    if (!file_) throw std::runtime_error("Не удалось открыть файл лога!");
}

void Logger::toggleMutex(bool enable) {
    useMutex_ = enable;
}

bool Logger::writeLine(const std::string& msg) {
    if (!useMutex_) {
        if (!file_) return false;
        file_ << msg << "\n";
        file_.flush();
        return true;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_) return false;
    file_ << msg << "\n";
    file_.flush();
    return true;
}
