#include <algorithm>
#include <cctype>
#include <chrono>
#include <future>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>
#include <set>
#include <semaphore>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace std;

struct SyncStream {
    static std::mutex& get_mutex() {
        static std::mutex m;
        return m;
    }
    std::lock_guard<std::mutex> lock;
    SyncStream(std::ostream& os) : lock(get_mutex()) {}
    template<typename T>
    std::ostream& operator<<(const T& msg) {
        return std::cout << msg;
    }
};

#define sync_cout SyncStream(std::cout)

void canceling_asynchronous_operations() {
  constexpr int CHECK_PERIOD_MS = 100;

  auto long_running_task = [&](int ms, const std::atomic_bool& cancellation_token)-> bool {
    while (ms > 0 && !cancellation_token) {
      ms -= CHECK_PERIOD_MS;
      std::this_thread::sleep_for(100ms);
    }
    return cancellation_token;
  };

  std::atomic_bool cancellation_token{false};
  std::cout << "Starting long running tasks..." << std::endl;

  std::packaged_task<bool(int, const std::atomic_bool&)> task1(long_running_task);
  std::future<bool> result1 = task1.get_future();
  std::jthread t1(std::move(task1), 500, std::ref(cancellation_token));

  std::packaged_task<bool(int, const std::atomic_bool&)> task2(long_running_task);
  std::future<bool> result2 = task2.get_future();
  std::jthread t2(std::move(task2), 1000, std::ref(cancellation_token));

  std::cout << "Cancelling tasks after 600ms" << std::endl;
  std::this_thread::sleep_for(600ms);
  cancellation_token = true;

  std::cout << "Task1, waiting for 500ms. Cancelled = " << std::boolalpha << result1.get() << std::endl;
  std::cout << "Task2, waiting for 1 second. Cancelled = " << std::boolalpha << result2.get() << std::endl;
}


void combineFunctions() {
  auto combineFunc = [&](std::promise<std::tuple<int, std::string>> combineProm) {
    try
    {
      // Thread to simulate computing a value.
      std::cout << "Starting computeThread..." << std::endl;
      auto computeVal = [](std::promise<int> prom) mutable {
        std::this_thread::sleep_for(1s);
        prom.set_value(42);
      };
      std::promise<int> computeProm;
      auto computeFut = computeProm.get_future();
      std::jthread computeThread(computeVal, std::move(computeProm));

      // Thread to simulate downloading a file.
      std::cout << "Starting dataThread..." << std::endl;
      auto fetchData = [](std::promise<std::string> prom) mutable {
        std::this_thread::sleep_for(2s);
        prom.set_value("data.txt");
      };
      std::promise<std::string> fetchProm;
      auto fetchFut = fetchProm.get_future();
      std::jthread dataThread(fetchData, std::move(fetchProm));
      
      combineProm.set_value({computeFut.get(), fetchFut.get()});
    }
    catch(...)
    {
      combineProm.set_exception(std::current_exception());
    }
    
  };
  std::promise<std::tuple<int, std::string>> combineProm;
  auto combineFuture = combineProm.get_future();
  std::jthread combineThread(combineFunc, std::move(combineProm));

  auto [data, file] = combineFuture.get();
  std::cout << "Value :" << data << " " << file << std::endl;

}


void async_first_example() {
  auto compute = [](unsigned taskId, int x, int y) -> int {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 200));
    sync_cout << "Running task " << taskId << std::endl;
    return std::pow(x, y);
  };

  vector<future<int>> futVec;
  for (int i = 0; i <= 10; i++) {
    futVec.emplace_back(async(compute, i, 2, i));
  }

  sync_cout << "Waiting in main thread" << endl;
  std::this_thread::sleep_for(1s);

  vector<int> results;
  for (auto& fut : futVec) {
    results.push_back(fut.get());
  }

  for (auto result : results) {
    std::cout << result << ' ';
  }
}

void limit_number_of_threads() {
  auto task = [](int id, std::counting_semaphore<>& sem) {
    sem.acquire();
    sync_cout << "Running task " << id << endl;
    std::this_thread::sleep_for(1s);
    sem.release();
  };

  const int total_tasks = 20;

  const int max_concurrent_tasks = std::thread::hardware_concurrency();
  std::counting_semaphore<> sem(max_concurrent_tasks);

  sync_cout << "Allowing only " << max_concurrent_tasks << " concurrent tasks to run " << total_tasks << " tasks" << endl;

  std::vector<std::future<void>> futures;
  for (int i = 0; i < total_tasks; i++) {
    futures.push_back(std::async(std::launch::async, task, i, std::ref(sem)));
  }

  for (auto& fut : futures) {
    fut.get();
  }
  std::cout << "All tasks completed" << std::endl;
}