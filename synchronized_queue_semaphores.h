#include <thread>
#include <vector>
#include <semaphore>

template <typename T>
class synchronized_queue {
public:
  explicit synchronized_queue(size_t size) :
    m_sem_empty(size),
    m_sem_full(0), 
    m_capacity(size), 
    buffer(size) {

  }

  void push(const T& item) {
    m_sem_empty.acquire();

    std::unique_lock<std::mutex> lock(m_mtx);
    m_buffer[m_tail] = item;
    m_tail = next(m_tail);
    lock.unlock();

    m_sem_full.release();
  }

  bool try_push(const T& item) {
    if (!m_sem_empty.try_acquire()) {
      return false;
    }

    std::unique_lock<std::mutex> lock(m_mtx);
    m_buffer[m_head] = item;
    m_head = next(m_head);
    lock.unlock();

    m_sem_full.release();

    return true;
  }

  void pop(T& item) {
    m_sem_full.acquire();

    std::unique_lock<std::mutex> lock(m_mtx);
    item = m_buffer[m_head];
    m_head = next(m_head);
    lock.unlock();

    m_sem_empty.release();
  }

  bool try_pop(T& item) {
    if (!m_sem_full.try_acquire()) {
      return false;
    }

    std::unique_lock<std::mutex> lock(m_mtx);
    item = m_buffer[m_head];
    m_head = next(m_head);
    lock.unlock();

    m_sem_empty.release();
    
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
  std::counting_semaphore<> m_sem_empty;
  std::counting_semaphore<> m_sem_full;

  size_t m_head{0};
  size_t m_tail{0};
  size_t m_capacity;
  std::vector<T> m_buffer; 
};