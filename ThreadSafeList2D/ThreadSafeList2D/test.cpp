#define TESTING_MODE  // comment it out in release

#include <algorithm>
#include <iostream>

#include "ThreadSafeList2D.h"

void FailWithMsg(const std::string& msg, int line) {
  std::cerr << "Test failed!\n";
  std::cerr << "[Line " << line << "] " << msg << std::endl;
  std::exit(EXIT_FAILURE);
}

#define ASSERT_TRUE(cond)                              \
  if (!(cond)) {                                       \
    FailWithMsg("Assertion failed: " #cond, __LINE__); \
  };

#define ASSERT_TRUE_MSG(cond, msg) \
  if (!(cond)) {                   \
    FailWithMsg(msg, __LINE__);    \
  };

#define ASSERT_EQUAL_MSG(cont1, cont2, msg)                       \
  ASSERT_TRUE_MSG(std::equal(std::begin(cont1), std::end(cont1),  \
                             std::begin(cont2), std::end(cont2)), \
                  msg)

#define REPEAT(count) for (size_t _iter = 0; _iter < count; ++_iter)

int main() {
  {  // can create list, check emptiness, check exceptions
    ThreadSafeList2D<int> list;

    ASSERT_TRUE(list.empty());
    ASSERT_TRUE(list.size() == 0);

    try {
      list.front();
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    } catch (AcceessViolation const& err) {
      ASSERT_TRUE("Access violation" == err.what());
    } catch (...) {
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    }

    try {
      list.back();
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    } catch (AcceessViolation const& err) {
      ASSERT_TRUE("Access violation" == err.what());
    } catch (...) {
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    }
  }

  {  // can push front and back, adequate size
    ThreadSafeList2D<int> list;

    list.push_front(1);
    ASSERT_TRUE(list.front() == 1);

    list.push_front(2);
    ASSERT_TRUE(list.front() == 2);

    ASSERT_TRUE(list.back() == 1);

    list.push_back(3);
    ASSERT_TRUE(list.back() == 3);

    list.push_back(4);
    ASSERT_TRUE(list.back() == 4);

    ASSERT_TRUE(list.size() == 4);
  }

  {  // remove unknown element exception, store connection (next/prev)
     // correctly, can remove successfully and store connection after it
    ThreadSafeList2D<int> list;

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);

    try {
      list.remove(5);
      FailWithMsg("Expected NotFound exception", __LINE__);
    } catch (ElementNotFound const& err) {
      ASSERT_TRUE("Element is not found" == err.what());
    } catch (...) {
      FailWithMsg("Expected NotFound exception", __LINE__);
    }

    ASSERT_TRUE(std::vector<int>({1, 2, 3, 4}) == list.get_fwd());
    ASSERT_TRUE(std::vector<int>({4, 3, 2, 1}) == list.get_bwd());

    list.remove(4);

    ASSERT_TRUE(std::vector<int>({1, 2, 3}) == list.get_fwd());
    ASSERT_TRUE(std::vector<int>({3, 2, 1}) == list.get_bwd());
  }

  {  // correct when becomes empty, empty list access throws exception
    ThreadSafeList2D<int> list;

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    ASSERT_TRUE(!list.empty());
    ASSERT_TRUE(list.size() == 3);

    list.remove(1);
    list.remove(2);
    list.remove(3);

    ASSERT_TRUE(list.empty());
    ASSERT_TRUE(list.size() == 0);

    try {
      list.front();
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    } catch (AcceessViolation const& err) {
      ASSERT_TRUE("Access violation" == err.what());
    } catch (...) {
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    }

    try {
      list.back();
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    } catch (AcceessViolation const& err) {
      ASSERT_TRUE("Access violation" == err.what());
    } catch (...) {
      FailWithMsg("Expected AcceessViolation exception", __LINE__);
    }
  }

  REPEAT(100) {  // correct multithreaded push_back
    ThreadSafeList2D<int> list;
    std::vector<std::thread> threads;
    for (int i = 1; i <= 10; ++i) {
      threads.push_back(
          std::thread(&ThreadSafeList2D<int>::push_back, &list, i));
    }
    for (int i = 0; i < threads.size(); i++) {
      threads.at(i).join();
    }
    auto list_res = list.get_fwd();
    std::sort(list_res.begin(), list_res.end());

    ASSERT_TRUE(std::vector<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) == list_res);
  }

  REPEAT(100) {  // correct multithreaded push_front
    ThreadSafeList2D<int> list;
    std::vector<std::thread> threads;
    for (int i = 1; i <= 10; ++i) {
      threads.push_back(
          std::thread(&ThreadSafeList2D<int>::push_front, &list, i));
    }
    for (int i = 0; i < threads.size(); i++) {
      threads.at(i).join();
    }
    auto list_res = list.get_fwd();
    std::sort(list_res.begin(), list_res.end());

    ASSERT_TRUE(std::vector<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) == list_res);
  }

  REPEAT(100) {  // correct multithreaded add and remove
    ThreadSafeList2D<int> list;
    std::vector<std::thread> threads_add;
    for (int i = 1; i <= 10; ++i) {
      threads_add.push_back(
          std::thread(&ThreadSafeList2D<int>::push_front, &list, i));
    }
    for (int i = 0; i < threads_add.size(); i++) {
      threads_add.at(i).join();
    }

    std::vector<std::thread> threads_remove;
    for (int i = 1; i <= 10; ++i) {
      threads_remove.push_back(
          std::thread(&ThreadSafeList2D<int>::remove, &list, i));
    }
    for (int i = 0; i < threads_remove.size(); i++) {
      threads_remove.at(i).join();
    }

    ASSERT_TRUE(list.empty());
    ASSERT_TRUE(list.size() == 0);
  }
  
  { // time measuring tests
    time_t timer;

    timer = clock();
    REPEAT(3) {  // simple add-remove
      ThreadSafeList2D<int> list;
      for (int i = 1; i <= 1000; ++i) {
        list.push_front(i);
      }
      for (int i = 1; i <= 1000; ++i) {
        list.remove(i);
      }
      ASSERT_TRUE(list.empty());
      ASSERT_TRUE(list.size() == 0);
    }
    std::cout << "Elapsed time for simple version: "
              << (double)(clock() - timer) / CLOCKS_PER_SEC << " seconds"
              << std::endl;

    timer = clock();
    REPEAT(3) {  // multithreaded push-remove
      ThreadSafeList2D<int> list;
      std::vector<std::thread> threads;
      for (int i = 1; i <= 1000; ++i) {
        threads.push_back(
            std::thread(&ThreadSafeList2D<int>::push_front, &list, i));
      }
      for (int i = 0; i < 1000; i++) {
        threads.at(i).join();
      }

      threads.clear();

      for (int i = 1; i <= 1000; ++i) {
        threads.push_back(
            std::thread(&ThreadSafeList2D<int>::remove, &list, i));
      }
      for (int i = 0; i < 1000; i++) {
        threads.at(i).join();
      }

      ASSERT_TRUE(list.empty());
      ASSERT_TRUE(list.size() == 0);
    }
    std::cout << "Elapsed time for multithreaded version: "
              << (double)(clock() - timer) / CLOCKS_PER_SEC << " seconds"
              << std::endl;    
  }

  return 0;
}
