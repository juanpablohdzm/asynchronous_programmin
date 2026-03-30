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
#include <list>
#include <set>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace std;

template <typename C>
bool search(const C& container, const std::string& target) {
  return std::find(container.begin(), container.end(), target) != container.end();
}

int execute() {
  std::vector<std::string> a = {"elephant", "giraffe", "lion", "zebra"};
  std::list<std::string> b = {"dfs", "giraffe", "lion", "zebra"};
  std::forward_list<std::string> c = {"dfs", "giraffe", "lion", "zebra"};
  std::set<std::string> d = {"few", "giraffe", "lion", "zebra"};

  std::string target = "elephant";

  /* create fut */
  /* found = fut1.get() || fut2.get() || ... || futN.get()*/
  return 0;
}