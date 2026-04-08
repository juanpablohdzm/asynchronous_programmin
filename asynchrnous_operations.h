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

int stage1(int x) {
  if (x < 0) throw std::runtime_error("Negative input not allowed");
  return x + 10;
}

int stage2(int x) {
  return x * 2;
}

int stage3(int x) {
  return x - 5;
}

void execute() {
  int input_value = 5;

  try {
    auto fut1 = std::async(std::launch::async, stage1, input_value);
    auto fut2 = std::async(std::launch::async, [&fut1]() {
      return stage2(fut1.get());
    });
    auto fut3 = std::async(std::launch::async, [&fut2](){
      return stage3(fut2.get());
    });
  } catch (const std::exception& ex) {
    std::cerr << "Exception caught: " << ex.what() << std::endl;
  }
}