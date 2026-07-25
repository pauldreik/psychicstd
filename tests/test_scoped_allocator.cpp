#include "psyassert.h"
#include <cstddef>
#include <scoped_allocator>
#include <vector>

template <typename T> struct counting_allocator {
  using value_type = T;

  std::size_t* allocated = nullptr;

  counting_allocator() = default;
  explicit counting_allocator(std::size_t* count) : allocated(count) {}
  template <typename U>
  counting_allocator(const counting_allocator<U>& other)
      : allocated(other.allocated) {}

  T* allocate(std::size_t count) {
    *allocated += count * sizeof(T);
    return static_cast<T*>(::operator new(count * sizeof(T)));
  }
  void deallocate(T* pointer, std::size_t count) {
    *allocated -= count * sizeof(T);
    ::operator delete(pointer);
  }

  template <typename U> bool operator==(const counting_allocator<U>&) const {
    return true;
  }
};

int main() {
  using inner_vector = std::vector<int, counting_allocator<int>>;
  using outer_allocator = counting_allocator<inner_vector>;
  using scoped_allocator = std::scoped_allocator_adaptor<outer_allocator>;

  std::size_t allocated = 0;
  {
    scoped_allocator allocator{outer_allocator{&allocated}};
    inner_vector* value = allocator.allocate(1);
    allocator.construct(value);
    const auto outer_bytes = allocated;
    value->push_back(42);
    psyassert(allocated > outer_bytes);
    allocator.destroy(value);
    allocator.deallocate(value, 1);
  }
  psyassert(allocated == 0);
}
