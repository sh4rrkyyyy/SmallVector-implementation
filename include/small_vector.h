#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>

template <class T> struct Helper {
  T *data;
  size_t size, capacity;
};
template <class T, typename = void> struct SmallVectorSize {
  char Base[sizeof(Helper<T>)];
  char FirstEl[sizeof(T)];
};
template <typename T> class VectorBase {
protected:
  T *data_;
  size_t size_ = 0;
  size_t capacity_ = 1;
  VectorBase() {}

public:
  VectorBase(const VectorBase &rhs) = delete;
  VectorBase(VectorBase &&) = delete;
  VectorBase &operator=(VectorBase &&) = delete;
  VectorBase &operator=(const VectorBase &) = delete;

  ~VectorBase() {
    if (!is_small()) {
      delete[] data_;
    }
  }
  bool is_small() const {
    return reinterpret_cast<const char *>(this) +
               offsetof(SmallVectorSize<T>, FirstEl) ==
           reinterpret_cast<const char *>(data_);
  }

  T &operator[](size_t idx) { return data_[idx]; }

  const T &operator[](size_t idx) const { return data_[idx]; }

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  void push_back(T &&elem) {
    if (is_small()) {
      if (size_ < capacity_) {
        data_[size_++] = std::move(elem);
      } else {
        capacity_ *= 2;
        auto tmp = new T[capacity_];
        std::copy(data_, data_ + size_, tmp);
        data_ = tmp;
        data_[size_++] = std::move(elem);
      }
    } else {
      if (size_ == capacity_) {
        capacity_ *= 2;
        auto tmp = new T[capacity_];
        std::copy(data_, data_ + size_, tmp);
        delete[] data_;
        data_ = tmp;
      }
      data_[size_++] = std::move(elem);
    }
  }
  void init(size_t capacity) {
    capacity_ = capacity;
    data_ = new T[capacity_];
  }

  void push_back(const T &elem) { push_back(std::move(T(elem))); }

  void pop_back() { --size_; }
};

template <typename T, size_t N> class SmallVector : public VectorBase<T> {
public:
  explicit SmallVector(size_t sz, const T &val = T()) {
    this->data_ = array_;
    this->capacity_ = N;
    this->size_ = 0;
    if (N < sz) {
      this->init(sz);
    }
    for (size_t i = 0; i < sz; ++i) {
      this->push_back(val);
    }
  }

  SmallVector(std::initializer_list<T> l) {
    this->data_ = array_;
    this->capacity_ = N;
    this->size_ = 0;
    if (N < l.size()) {
      this->init(l.size());
    }
    for (auto &el : l) {
      this->push_back(std::move(el));
    }
  }

  template <typename Iter> SmallVector(Iter begin, Iter end) {
    this->data_ = array_;
    this->capacity_ = N;
    this->size_ = 0;
    if (N < (size_t)(std::distance(begin, end))) {
      this->init((size_t)(std::distance(begin, end)));
    }
    for (auto it = begin; it != end; ++it) {
      this->push_back(std::move(*it));
    }
  }

  SmallVector() {
    this->data_ = array_;
    this->capacity_ = N;
    this->size_ = 0;
  }
  SmallVector(const SmallVector &rhs) {
    this->size_ = rhs.size();
    this->capacity_ = rhs.capacity();
    if (!rhs.is_small()) {
      this->data_ = new T[this->capacity_];
      std::copy(rhs.data_, rhs.data_ + rhs.size(), this->data_);
    } else {
      std::copy(rhs.array_, rhs.array_ + rhs.size(), this->array_);
      this->data_ = this->array_;
    }
  }

  SmallVector(SmallVector &&rhs) {
    if (!rhs.is_small()) {
      this->data_ = rhs.data_;
      this->capacity_ = rhs.capacity_;
      this->size_ = rhs.size_;
      rhs.data_ = nullptr;
    } else {
      std::move(rhs.array_, rhs.array_ + rhs.size(), this->array_);
      this->data_ = this->array_;
      this->capacity_ = N;
      this->size_ = rhs.size_;
    }
  }

  SmallVector &operator=(const SmallVector &rhs) {
    if (this != &rhs) {
      if (!this->is_small()) {
        delete[] this->data_;
      }
      this->size_ = rhs.size();
      this->capacity_ = rhs.capacity();
      if (!rhs.is_small()) {
        this->data_ = new T[this->capacity_];
        std::copy(rhs.data_, rhs.data_ + rhs.size(), this->data_);
      } else {
        std::copy(rhs.array_, rhs.array_ + rhs.size(), this->array_);
      }
    }
    return *this;
  }
  SmallVector &operator=(SmallVector &&rhs) {
    if (this != &rhs) {
      if (!this->is_small()) {
        delete[] this->data_;
      }
      if (!rhs.is_small()) {
        this->data_ = rhs.data_;
        this->capacity_ = rhs.capacity_;
        this->size_ = rhs.size_;
        rhs.data_ = nullptr;
      } else {
        std::move(rhs.array_, rhs.array_ + rhs.size(), this->array_);
        this->data_ = this->array_;
        this->capacity_ = N;
        this->size_ = rhs.size_;
      }
    }
    return *this;
  }

  bool reset_small() {
    if (this->size() <= N) {
      if (!this->is_small()) {
        std::copy(this->data_, this->data_ + this->size(), array_);
        delete[] this->data_;
        this->data_ = array_;
        return true;
      }
      return true;
    }
    return false;
  }

private:
  T array_[N];
};
