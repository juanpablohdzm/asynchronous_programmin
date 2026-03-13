#include <iostream>
#include <syncstream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <chrono>
#include <shared_mutex>
#include <condition_variable>
#include "barriers_and_latches.h"

using namespace std::chrono_literals;
using namespace std::chrono;

#define sync_cout std::osyncstream(std::cout)

int counter = 0;

int main(int argc, const char* const argv[]) {

  barriers_and_latches::first_latch_example();
  return 0;
}