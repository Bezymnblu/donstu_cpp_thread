#include <iostream>
#include <vector>
#include <thread>
#include <sstream>
#include <future>
#include <chrono>
#include <unistd.h>
#include <sys/syscall.h>
#include "threadfuncs.h"

Logger logger("output.log");

void counterWorker() {
    for (int i = 0; i < 100000; ++i) {
        normalCounter++;
        atomicCounter++;
    }
}

std::string valueWorker(ThreadArgs& args) {
    int counter = 0;
    for (int i = 0; i < COUNT_ITERATIONS; ++i) {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return "Поток " + args.tag + " завершил " + std::to_string(counter) + " шагов.";
}

std::mutex cpMutex;
std::condition_variable cv;
int buffer = -1;
bool ready = false;
bool done = false;

void producer() {
    for (int i = 1; i <= 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        {
            std::lock_guard<std::mutex> lock(cpMutex);
            buffer = i;
            ready = true;
            logger.writeLine("[Producer] Записал в буфер: " + std::to_string(i));
        }
        cv.notify_one(); 
    }
    {
        std::lock_guard<std::mutex> lock(cpMutex);
        done = true;
    }
    cv.notify_one();
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(cpMutex);
        cv.wait(lock, [] { return ready || done; });

        if (ready) {
            logger.writeLine("[Consumer] Прочитал из буфера: " + std::to_string(buffer));
            ready = false;
        }
        if (done && !ready) {
            logger.writeLine("[Consumer] Конец работы. Выход.");
            break;
        }
    }
}

int main() {
    logger.writeLine("main: старт. PID: " + std::to_string(getpid()));

    std::vector<std::thread> threads;
    std::vector<ThreadArgs> args(COUNT_THREADS);

    for (int i = 0; i < COUNT_THREADS; ++i) {
        std::ostringstream oss;
        oss << "T" << i;
        args[i].id = i;
        args[i].tag = oss.str();
    }

    for (int i = 0; i < COUNT_THREADS; ++i) {
        threads.emplace_back(std::thread([i, &args]() {
            for (int j = 0; j < COUNT_ITERATIONS; ++j) {
                std::ostringstream ss;
                ss << "[Нить: " << args[i].tag 
                   << "] Шаг: " << j
                   << " | Kernel TID: " << syscall(SYS_gettid)
                   << " | std::thread::id: " << std::this_thread::get_id();
                logger.writeLine(ss.str());
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }));
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::vector<std::thread> counterThreads;
    for (int i = 0; i < 4; ++i) {
        counterThreads.emplace_back(counterWorker);
    }
    for (auto& t : counterThreads) t.join();

    logger.writeLine("--- Результаты счетчиков ---");
    logger.writeLine("Обычный int (Data Race): " + std::to_string(normalCounter));
    logger.writeLine("std::atomic<int>: " + std::to_string(atomicCounter));

    ThreadArgs asyncArgs{88, "AsyncFuture"};
    std::future<std::string> fut = std::async(std::launch::async, valueWorker, std::ref(asyncArgs));
    logger.writeLine("--- Данные из future ---");
    logger.writeLine(fut.get());

    logger.writeLine("--- Старт Producer-Consumer ---");
    std::thread tProd(producer);
    std::thread tCons(consumer);
    tProd.join();
    tCons.join();

    logger.writeLine("main: работа успешно завершена.");
    return 0;
}
