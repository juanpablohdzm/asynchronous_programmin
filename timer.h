#include <chrono>
#include <functional>
#include <iostream>
#include <syncstream>
#include <thread>

using namespace std::chrono_literals;
using namespace std::chrono;

#define sync_cout std::osyncstream(std::cout)

template<typename Interval>
class Timer {
public:
  using Callback = std::function<void(void)>;
  Timer(const Interval interval, Callback callback) {
    auto value = duration_cast<milliseconds>(interval);
    sync_cout << "Timer: Starting with interval of " << value << std::endl;

    t = std::jthread([this, callback = std::move(callback), interval](std::stop_token stop_token) mutable {
      while (!stop_token.stop_requested()) {
        sync_cout << "Timer: Running callback " << val.load() << std::endl;
        val++;
        callback();
        sync_cout << "Timer: Sleeping... \n";
        std::this_thread::sleep_for(interval);
      }
      sync_cout << "Timer: Exit \n";
    });
  }

  void stop() {
    t.request_stop();
  }
private:
  std::jthread t;
  std::atomic_int32_t val{0};
};