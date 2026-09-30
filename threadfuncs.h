#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <atomic>

inline constexpr int COUNT_THREADS = 4;
inline constexpr int COUNT_ITERATIONS = 20; 

struct ThreadArgs {
    int id;
    std::string tag;
};

class Logger {
public:
    explicit Logger(const std::string& filename);
    bool writeLine(const std::string& msg);
    void toggleMutex(bool enable);

private:
    std::ofstream file_;
    std::mutex mutex_;
    bool useMutex_ = true;
};

extern int normalCounter;
extern std::atomic<int> atomicCounter;
