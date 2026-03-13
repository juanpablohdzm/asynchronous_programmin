#include <thread>
#include <vector>

template <typename T>
class synchronized_queue {
public:
  explicit synchronized_queue(size_t size) : 
    m_capacity(size), 
    buffer(m_capacity) {

  }

  void push(const T& item) {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_not_full_.wait(lock, [this] () { return !is_full(); });

    m_buffer[m_tail] = item;
    m_tail = next(m_tail);

    lock.unlock();

    m_not_empty.notify_one();
  }

  bool try_push(const T& item) {
    std::unique_lock<std::mutex> lock(m_mtx, std::try_lock);
    if (!lock || is_full()) {
      return false;
    }

    m_buffer[m_tail] = item;
    m_tail = next(m_tail);

    lock.unlock();

    m_not_empty.notify_one();

    return true;
  }

  void pop(T& item) {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_not_empty.wait(lock, [this]() {return !is_empty()});

    item = m_buffer[m_head];
    m_head = next(m_head);

    lock.unlock();
    m_not_full.notify_one();
  }

  bool try_pop(T& item) {
    std::unique_lock<std::mutex> lock(m_mtx, std::try_lock);
    if (!lock || is_empty()) {
      return false;
    }

    item = m_buffer[m_head];
    m_head = next(m_head);

    lock.unlock();
    m_not_full.notify_one();

    return true;
  }


private:

  size_t next(size_t index) {
    return (index + 1) % m_capacity;
  }

  bool is_full() const {
    return next(m_tail) == m_head;
  }

  bool is_empty() const  {
    return m_tail == m_head;
  }

  std::mutex m_mtx;
  std::condition_variable m_not_empty;
  std::condition_variable m_not_full;

  size_t m_head{0};
  size_t m_tail{0};
  size_t m_capacity;
  std::vector<T> m_buffer; 
};