#pragma once
#include <exception>
#include <mutex>

#ifdef TESTING_MODE
#include <vector>
#endif

/**
 * \struct ElementNotFound
 *
 *
 * \brief Simple struct for ElementNotFound exception
 *
 * The exception returns string "Element is not found".
 *
 *
 * \author $Author: Liliya Makhmutova $
 *
 * \version $Revision: 1.0 $
 *
 * \date $Date: 2021/01/16 00:00:00 $
 */
struct ElementNotFound : public std::exception {
  /// main method that returns message
  const char* what() const throw() { return "Element is not found"; }
};

/**
 * \struct AcceessViolation
 *
 *
 * \brief Simple struct for AcceessViolation exception
 *
 * The exception returns string "Access violation".
 *
 *
 * \author $Author: Liliya Makhmutova $
 *
 * \version $Revision: 1.0 $
 *
 * \date $Date: 2021/01/16 00:00:00 $
 */
struct AcceessViolation : public std::exception {
  /// main method that returns message
  const char* what() const throw() { return "Access violation"; }
};

/**
 * \class ThreadSafeList2D
 *
 *
 * \brief Implements thread safe doubly linked list data structure.
 *
 * \tparam T Class to store in the linked list.
 *
 * ThreadSafeList2D uses std::lock_guard to achieve thread safety.
 * All of the operations are standard for doubly linked list data structure.
 * Copy constructor and copy assignment operations are restricted (deleted) for
 * the sake of simplicity and to avoid pointer problems.
 *
 *
 * \author Liliya Makhmutova
 *
 * \version 1.0
 *
 * \date $Date: 2021/01/19 00:00:00 $
 */
template <class T>
class ThreadSafeList2D {
  /**
   * \struct Node
   *
   *
   * \brief Simple structure to represent node in the list.
   *
   * \tparam T Class to store in the linked list.
   *
   * Each node has pointer to previous and next node, it also stores a value.
   *
   *
   * \author $Author: Liliya Makhmutova $
   *
   * \version $Revision: 1.0 $
   *
   * \date $Date: 2021/01/16 00:00:00 $
   */
  struct Node {
    explicit Node(T value, Node* prev)
        : value(value), prev(prev), next(nullptr) {}
    explicit Node(T value) : value(value), prev(nullptr), next(nullptr) {}
    struct Node* prev;
    T value;
    struct Node* next;
  };

 public:
  /// Simple constructor, initially list is empty
  ThreadSafeList2D() noexcept : head(nullptr), tail(nullptr), size_(0) {}

  /// Copy constructor is disabled
  ThreadSafeList2D(const ThreadSafeList2D<T>& rhs) = delete;
  /// Copy assignment is disabled
  ThreadSafeList2D& operator=(const ThreadSafeList2D<T>& rhs) = delete;

  /// Recursively delete all the nodes in destructor
  ~ThreadSafeList2D() {
    Node* tmp = nullptr;
    while (head) {
      tmp = head;
      head = head->next;
      delete tmp;
    }
    head = nullptr;
  }

  /** \brief Method that returns the value of the first element of the linked
   * list.
   *
   *
   * \return Outputs value of the first node.
   *
   * \warning this finction uses mutex lock_guard and throws AcceessViolation
   * exception in case of empty list.
   */
  T front() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!head) {
      throw AcceessViolation();
    }
    return head->value;
  }

  /** \brief Method that returns the value of the last element of the linked
   * list.
   *
   *
   * \return Outputs value of the last node.
   *
   * \warning this finction uses mutex lock_guard and throws AcceessViolation
   * exception in case of empty list.
   */
  T back() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!tail) {
      throw AcceessViolation();
    }
    return tail->value;
  }

  /** \brief Method that returns the size of the linked list.
   *
   * \return Outputs actual size of list.
   *
   * \warning this finction uses mutex lock_guard.
   *
   * \note This method is guaranteed not to throw an exception.
   */
  size_t size() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_;
  }

  /** \brief Method that returns true if list is empty.
   *
   * It checks whether size == 0.
   *
   * \return Boolean value that indicates that list is empty.
   * \warning this finction uses mutex lock_guard.
   * \note This method is guaranteed not to throw an exception.
   */
  bool empty() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_ == 0;
  }

  /** \brief Method inserts element at the beginning.
   * \param val value that will be added to the list
   *
   * It creates new element with a value passed.
   * Then updates head and links to the existing list.
   *
   *
   * \warning this finction uses mutex lock_guard.
   * \note This method is guaranteed not to throw an exception.
   */
  void push_front(T val) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    Node* node = new Node(val);
    Node* tmp = head;
    if (head == nullptr) {  // empty list
      head = tail = node;
    } else {
      node->next = head;
      head = node;
      node->next->prev = node;
    }
    size_++;
  }

  /** \brief Method inserts element at the end.
   * \param val value that will be added to the list
   *
   * It creates new element with a value passed.
   * Then updates tail and links to the existing list.
   *
   *
   * \warning this finction uses mutex lock_guard.
   * \note This method is guaranteed not to throw an exception.
   */
  void push_back(T val) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    Node* node = new Node(val, tail);
    if (head == nullptr) {  // empty list
      head = tail = node;
    } else {
      tail->next = node;
      tail = node;
    }
    size_++;
  }

  /** \brief Method removes element from the list by value.
   * \param val value that will be removed
   *
   * It searches the node with value val. Then depending on where the node is
   * located, it frees memory and fixes list structure.
   *
   *
   * \warning this finction uses mutex lock_guard and throws ElementNotFound.
   */
  void remove(T val) {
    std::lock_guard<std::mutex> lock(mutex_);

    Node* found_node = find(val);
    Node* tmp = head;

    if (found_node) {  // nothing to delete otherwise
      if (found_node == head) {
        head = head->next;
        if (head) {
          head->prev = nullptr;
        } else {
          tail = nullptr;
        }
        delete found_node;
      } else if (found_node == tail) {
        tail = tail->prev;
        if (tail) {
          tail->next = nullptr;
        }
        delete found_node;
      } else {
        Node* prev_node = found_node->prev;
        Node* next_node = found_node->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        delete found_node;
      }
      size_--;
    } else {
      throw ElementNotFound();
    }
  }

#ifdef TESTING_MODE
  /// Need to iterate forward the list and get vector of list values (for
  /// testing purposes only).
  std::vector<T> get_fwd() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<T> result;
    Node* node = head;

    while (node != nullptr) {
      result.push_back(node->value);
      node = node->next;
    }

    return result;
  }

  /// Need to iterate backward the list and get vector of list values (for
  /// testing purposes only).
  std::vector<T> get_bwd() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<T> result;
    Node* node = tail;

    while (node != nullptr) {
      result.push_back(node->value);
      node = node->prev;
    }

    return result;
  }
#endif

 private:
  Node* head;
  Node* tail;
  size_t size_;
  mutable std::mutex mutex_;  /// to use std::lock_guard

  /** \brief Method that finds element in the list by value.
   * \param val value that will be found
   *
   * It searches the node with value val iterating the list forward.
   *
   * \return pointer to the found node.
   *
   * \note This method is guaranteed not to throw an exception.
   */
  Node* find(T val) noexcept {
    Node* node = head;
    while (node != nullptr) {
      if (node->value == val) {
        return node;
      }
      node = node->next;
    }
    return nullptr;
  }
};