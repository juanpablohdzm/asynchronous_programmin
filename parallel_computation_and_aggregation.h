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

int square(int x) {
  return x * x;
}

int sum_results(std::vector<std::future<int>>& futsVec) {
  int sum = 0;
  for (auto& fut : futsVec) {
    sum += fut.get();
  }
  return sum;
}

int average_squares(int n) {
  std::vector<std::future<int>> futsVec;
  for (int i = 1; i <= n; i++) {
    futsVec.push_back(std::async(std::launch::async, square, i));
  }
  return double(sum_results(futsVec))/n;
}

int execute() {
  int N = 100;
  std::cout << "Sum of squares for N = " << N << "is" << average_squares(N) << endl;
  return 0;
}