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
#include <mutex>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace std;

// Fallback for std::osyncstream
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

int counter = 0;

template <typename Func>
class Task {
  public:
    Task(int id, Func& func) : id_(id), func_(func), has_dependency_(false) {
      sync_cout << "Task " << id_ << " constructured without dependencies" << endl;
      fut_ = prom_.get_future().share();
    }

    template <typename... Futures>
    Task(int id, Func& func, Futures&&... futures)
      : id_(id), func_(func), has_dependency_(true) {
        sync_cout << "Task: " << id_ << " constructed with dependencies" << endl;
        fut_ = prom_.get_future().share();
        add_dependencies(std::forward<Futures>(futures)...);
    }

    std::shared_future<void> get_dependency() {
      return fut_;
    }

    void operator()() {
      sync_cout << "Running task " << id_ << endl;
      wait_completion();
      func_();
      sync_cout << "Signaling completion of task " << id_ << endl;
      prom_.set_value();
    }

  private:

    template <typename... Futures>
    void add_dependencies(Futures&&... futures) {
      (deps_.push_back(futures), ...);
    }

    void wait_completion() {
      sync_cout << "Waiting completion for task " << id_ << endl;
      if (!deps_.empty()) {
        for (auto& fut : deps_) {
          if (fut.valid()) {
            sync_cout << "Fut valid so getting value in task " << id_ << endl;
            fut.get();
          }
        }
      }
    }

    int id_;
    Func& func_;
    std::promise<void> prom_;
    std::shared_future<void> fut_;
    std::vector<std::shared_future<void>> deps_;
    bool has_dependency_;
};

int execute(int argc, const char* const argv[]) {

  auto sleep1s = []() { std::this_thread::sleep_for(1s);};
  auto sleep2s = []() { std::this_thread::sleep_for(2s);};

  Task task1(1, sleep1s);
  Task task2(2, sleep2s, task1.get_dependency());
  Task task3(3, sleep1s, task2.get_dependency());
  Task task4(4, sleep2s, task2.get_dependency());
  Task task5(5, sleep2s, task3.get_dependency(), task4.get_dependency());

  sync_cout << "Starting the pipeline..." << std::endl;
  task1();
  task2();
  task3();
  task4();
  task5();

  sync_cout << "Waiting for the pipeline to finish" << endl;
  auto finish_pipeline_fut = task5.get_dependency();
  finish_pipeline_fut.get();
  sync_cout << "All done!" << std::endl;
  
  return 0;
}