#include <thread>
#include <latch>
#include <barrier>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;
using namespace std::chrono;

namespace barriers_and_latches {
  void first_latch_example () {

    int sum = 0;

    std::latch map_latch {3};
    std::vector<int> nums {1,2,3,4,5,6,7,8,9};
    auto map_thread = [&](std::vector<int>& numbers, int start, int end) {
      for (int i = start; i < end; i++) {
        numbers[i] *= 2;
        std::this_thread::sleep_for(1s);
      }
      std::cout << "Done:" << std::endl;
      map_latch.count_down();
    };

    std::latch reduce_latch {1};
    auto reduce_thread = [&](const std::vector<int>& numbers, int& sum) {
      map_latch.wait();

      sum = std::accumulate(numbers.begin(), numbers.end(), 0);

      reduce_latch.count_down();
    };
    
    std::jthread j0(reduce_thread, std::ref(nums), std::ref(sum));
    std::jthread j1(map_thread, std::ref(nums), 0, 3);
    std::jthread j2(map_thread, std::ref(nums), 3, 6);
    std::jthread j3(map_thread, std::ref(nums), 6, 9);

    reduce_latch.wait();
    std::cout << "All threads finished. The sum is: " << sum << std::endl;
  }

  void first_barrier_example() {

    int sum = 0;

    std::barrier map_barrier {3};
    std::vector<int> nums {1,2,3,4,5,6,7,8,9};
    std::vector<int> results{0,0,0};

    auto worker_thread = [&](std::vector<int>& numbers, int start, int end, int id) {
      for (int i = start; i < end; i++) {
        numbers[i] *= 2;
        std::this_thread::sleep_for(1s);
      }
      map_barrier.arrive_and_wait();

      for (int i = start; i < end; i++) {
        results[id] += numbers[i];
      }
      map_barrier.arrive();
    };
  }
}