#include <atomic>
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>
#include <future>

template <typename T>
class ThreadSafeQueue {
  public: 
  void push(T value) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(value));
    cond_var_.notify_one();
  }

  T pop(){
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [&]{
      return !queue_.empty();
    });
    T value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

  private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
};

namespace ThreadSafeQueueExample{
  using TaskQueue = ThreadSafeQueue<std::future<int>>;
  void producer(TaskQueue& queue, int val) {
    std::promise<int> prom;
    auto fut = prom.get_future();
    queue.push(std::move(fut));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % MAX_WAIT));

    prom.set_value(val);
  }

  void consumer(TaskQueue& queue) {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % MAX_WAIT));

    std::future<int> fut = queue.pop();
    try
    {
      int result = fut.get();
      std::cout << "Result: " << result << std::endl;
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    
  }
  
}