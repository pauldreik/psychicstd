#include "psyassert.h"
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct Base {
  virtual ~Base() = default;
  int x = 42;
};
struct Derived : Base {
  int y = 99;
};

struct Forwarded {
  int kind;
  explicit Forwarded(int&) : kind(1) {}
  explicit Forwarded(int&&) : kind(2) {}
};

struct ThrowingConstruction {
  static inline int alive;
  static inline int copies;

  ThrowingConstruction() { ++alive; }
  ThrowingConstruction(const ThrowingConstruction&) {
    if (++copies == 2)
      throw 1;
    ++alive;
  }
  ~ThrowingConstruction() { --alive; }
};

template <typename T> struct counting_allocator {
  using value_type = T;
  int* allocations;

  explicit counting_allocator(int* count) : allocations(count) {}
  template <typename U>
  counting_allocator(const counting_allocator<U>& other)
      : allocations(other.allocations) {}

  T* allocate(std::size_t n) {
    ++*allocations;
    return std::allocator<T>{}.allocate(n);
  }
  void deallocate(T* p, std::size_t n) {
    --*allocations;
    std::allocator<T>{}.deallocate(p, n);
  }
};

struct Allocated {
  int value;
  explicit Allocated(int v) : value(v) {}
};

struct AllocatorAware {
  using allocator_type = std::allocator<int>;
};

static_assert(
    std::uses_allocator_v<AllocatorAware, std::allocator<AllocatorAware>>);

struct CountingDeleter {
  int* count;
  void operator()(Derived* ptr) const {
    ++*count;
    delete ptr;
  }
};

static void test_forwarding() {
  int value = 1;
  alignas(Forwarded) unsigned char storage[sizeof(Forwarded)];
  auto* raw = reinterpret_cast<Forwarded*>(storage);
  auto* constructed = std::construct_at(raw, value);
  psyassert(constructed->kind == 1);
  std::destroy_at(constructed);

  std::allocator<Forwarded> alloc;
  auto* allocated = alloc.allocate(1);
  std::allocator_traits<decltype(alloc)>::construct(alloc, allocated,
                                                    static_cast<int&&>(value));
  psyassert(allocated->kind == 2);
  std::allocator_traits<decltype(alloc)>::destroy(alloc, allocated);
  alloc.deallocate(allocated, 1);

  psyassert(std::make_unique<Forwarded>(value)->kind == 1);
  psyassert(std::make_shared<Forwarded>(static_cast<int&&>(value))->kind == 2);

  // make_unique<T[]>(n) must select the array overload (value-initialized),
  // not the variadic single-object one (cmake regression).
  auto arr = std::make_unique<char[]>(16);
  psyassert(arr[0] == 0 && arr[15] == 0);
  psyassert(std::assume_aligned<alignof(Forwarded)>(raw) == raw);

  alignas(Forwarded) unsigned char range_storage[2 * sizeof(Forwarded)];
  auto* range = reinterpret_cast<Forwarded*>(range_storage);
  std::construct_at(range, value);
  std::construct_at(range + 1, value);
  std::destroy(range, range + 2);
}

static void test_overaligned_allocation() {
  // std::allocator<T>::allocate must request extended alignment for T
  // over-aligned past __STDCPP_DEFAULT_NEW_ALIGNMENT__ (typically 16) --
  // the plain, unaligned operator new has no way to honor a stricter
  // alignment, so silently using it is undefined behavior (the real-world
  // trigger is any SIMD-vectorizable type, e.g. Eigen's fixed-size
  // vectors, going through std::vector/make_shared/etc.).
  struct alignas(64) over_aligned {
    char payload[64];
  };
  std::allocator<over_aligned> aligned_alloc;
  auto* aligned_ptr = aligned_alloc.allocate(4);
  psyassert(reinterpret_cast<std::uintptr_t>(aligned_ptr) % 64 == 0);
  aligned_alloc.deallocate(aligned_ptr, 4);

  std::vector<over_aligned> aligned_vector(4);
  for (auto& item : aligned_vector)
    psyassert(reinterpret_cast<std::uintptr_t>(&item) % 64 == 0);
}

static void test_unique_ptr_equality() {
  auto first = std::make_unique<Derived>();
  std::unique_ptr<Base> alias(first.get());
  psyassert(first == alias);
  alias.release();

  auto second = std::make_unique<Derived>();
  psyassert(first != second);

  std::unique_ptr<int[]> array1(new int[1]);
  std::unique_ptr<int[]> array2;
  psyassert(array1 != array2);
}

static void test_allocate_shared() {
  int allocations = 0;
  {
    auto value = std::allocate_shared<Allocated>(
        counting_allocator<void>(&allocations), 42);
    psyassert(value->value == 42);
    psyassert(allocations == 1);
  }
  psyassert(allocations == 0);
}

static void test_converting_copy_ctor() {
  std::shared_ptr<Derived> d = std::make_shared<Derived>();
  psyassert(d.use_count() == 1);

  std::shared_ptr<Base> b = d;
  psyassert(b.use_count() == 2);
  psyassert(d.use_count() == 2);

  d.reset();
  psyassert(b.use_count() == 1);
  psyassert(b->x == 42);
}

static void test_owner_before() {
  auto owner = std::make_shared<Derived>();
  auto other_owner = std::make_shared<Derived>();
  std::shared_ptr<Base> converted = owner;
  std::weak_ptr<Derived> weak = owner;
  std::weak_ptr<Derived> other_weak = other_owner;
  std::weak_ptr<Base> converted_weak = converted;
  psyassert(!owner.owner_before(converted));
  psyassert(!converted.owner_before(weak));
  psyassert(!weak.owner_before(converted));
  psyassert(!weak.owner_before(converted_weak));
  bool owner_first = owner.owner_before(other_owner);
  psyassert(owner_first != other_owner.owner_before(owner));
  psyassert(owner_first == weak.owner_before(other_owner));
  psyassert(owner_first == owner.owner_before(other_weak));
  psyassert(owner_first == weak.owner_before(other_weak));
}

static void test_smart_pointer_relations_and_output() {
  auto first = std::make_unique<int>(1);
  auto second = std::make_unique<int>(2);
  psyassert((first < second) != (second < first));

  std::ostringstream shared_output;
  auto shared = std::make_shared<int>(1);
  auto other_shared = std::make_shared<int>(2);
  psyassert((shared < other_shared) != (other_shared < shared));
  shared_output << shared;
  std::ostringstream raw_output;
  raw_output << shared.get();
  psyassert(shared_output.str() == raw_output.str());

  auto text = std::make_unique<char[]>(2);
  text[0] = 'x';
  text[1] = '\0';
  std::ostringstream text_output;
  text_output << text;
  psyassert(text_output.str() == "x");
}

static void test_converting_ctor_from_prvalue() {
  std::shared_ptr<Base> b(std::make_shared<Derived>());
  psyassert(b.use_count() == 1);
  psyassert(b->x == 42);

  std::shared_ptr<Base> b2 = b;
  psyassert(b.use_count() == 2);
  b.reset();
  psyassert(b2.use_count() == 1);
  psyassert(b2->x == 42);
}

static void test_const_pointer_cast() {
  auto mutable_ptr = std::make_shared<int>(42);
  std::shared_ptr<const int> const_ptr = mutable_ptr;
  auto cast = std::const_pointer_cast<int>(const_ptr);
  *cast = 7;
  psyassert(*mutable_ptr == 7);
  psyassert(cast.use_count() == 3);

  auto* raw = new int(9);
  std::shared_ptr<const int> source(raw);
  auto moved = std::const_pointer_cast<int>(
      static_cast<std::shared_ptr<const int>&&>(source));
  psyassert(!source);
  psyassert(moved.get() == raw);
  psyassert(moved.use_count() == 1);
}

static void test_uninitialized_exception_cleanup() {
#if defined(__cpp_exceptions)
  {
    ThrowingConstruction::alive = 0;
    ThrowingConstruction::copies = 0;
    ThrowingConstruction input[3];
    alignas(ThrowingConstruction) unsigned char
        storage[sizeof(ThrowingConstruction) * 3];
    auto* output = reinterpret_cast<ThrowingConstruction*>(storage);
    try {
      std::uninitialized_copy_n(input, 3, output);
      psyassert(false);
    } catch (int) {
    }
    psyassert(ThrowingConstruction::alive == 3);
    ThrowingConstruction::copies = 0;
    alignas(ThrowingConstruction) unsigned char
        range_storage[sizeof(ThrowingConstruction) * 3];
    auto* range_output = reinterpret_cast<ThrowingConstruction*>(range_storage);
    try {
      std::uninitialized_copy(input, input + 3, range_output);
      psyassert(false);
    } catch (int) {
    }
    psyassert(ThrowingConstruction::alive == 3);
  }
  psyassert(ThrowingConstruction::alive == 0);
  {
    ThrowingConstruction::copies = 0;
    ThrowingConstruction value;
    alignas(ThrowingConstruction) unsigned char
        storage[sizeof(ThrowingConstruction) * 3];
    auto* output = reinterpret_cast<ThrowingConstruction*>(storage);
    try {
      std::uninitialized_fill_n(output, 3, value);
      psyassert(false);
    } catch (int) {
    }
    psyassert(ThrowingConstruction::alive == 1);
    ThrowingConstruction::copies = 0;
    alignas(ThrowingConstruction) unsigned char
        range_storage[sizeof(ThrowingConstruction) * 3];
    auto* range_output = reinterpret_cast<ThrowingConstruction*>(range_storage);
    try {
      std::uninitialized_fill(range_output, range_output + 3, value);
      psyassert(false);
    } catch (int) {
    }
    psyassert(ThrowingConstruction::alive == 1);
  }
  psyassert(ThrowingConstruction::alive == 0);
#endif
}

struct SelfShared : std::enable_shared_from_this<SelfShared> {
  std::shared_ptr<SelfShared> self() { return shared_from_this(); }
};

static void test_enable_shared_from_this() {
  auto owner = std::make_shared<SelfShared>();
  auto self = owner->self();
  psyassert(self.get() == owner.get());
  psyassert(owner.use_count() == 2);

  SelfShared unowned;
  bool threw = false;
  try {
    (void)unowned.self();
  } catch (const std::bad_weak_ptr&) {
    threw = true;
  }
  psyassert(threw);

  struct keep_alive {
    void operator()(SelfShared*) const {}
  };
  SelfShared rebound;
  {
    std::shared_ptr<SelfShared> first(&rebound, keep_alive{});
    psyassert(rebound.self().use_count() == 2);
  }
  {
    std::shared_ptr<SelfShared> second(&rebound, keep_alive{});
    auto rebound_self = rebound.self();
    psyassert(rebound_self.get() == &rebound);
    psyassert(second.use_count() == 2);
  }
}

static void test_shared_ptr_from_unique_ptr() {
  int deletes = 0;
  std::unique_ptr<Derived, CountingDeleter> unique(new Derived, {&deletes});
  std::shared_ptr<Base> shared = std::move(unique);
  psyassert(!unique);
  psyassert(shared->x == 42);
  psyassert(deletes == 0);

  std::unique_ptr<Derived, CountingDeleter> replacement(new Derived,
                                                        {&deletes});
  shared = std::move(replacement);
  psyassert(!replacement);
  psyassert(shared->x == 42);
  psyassert(deletes == 1);
  shared.reset();
  psyassert(deletes == 2);
}

static void test_shared_ptr_custom_deleter() {
  int deletes = 0;
  std::shared_ptr<Derived> shared;
  shared.reset(new Derived, CountingDeleter{&deletes});

  auto* deleter = std::get_deleter<CountingDeleter>(shared);
  psyassert(deleter);
  psyassert(deleter->count == &deletes);
  psyassert(!std::get_deleter<std::default_delete<Derived>>(shared));

  shared.reset();
  psyassert(deletes == 1);
}

static void test_shared_ptr_concurrent_destruction() {
  auto owner = std::make_shared<int>(42);
  std::thread first([copy = owner] {});
  std::thread second([copy = owner] {});
  first.join();
  second.join();
  psyassert(*owner == 42);
}

static void test_shared_ptr_atomic_access() {
  std::shared_ptr<int> value = std::make_shared<int>(1);
  std::shared_ptr<int> loaded = std::atomic_load(&value);
  psyassert(*loaded == 1);
  psyassert(loaded.use_count() == 2);

  std::atomic_store(&value, std::make_shared<int>(2));
  psyassert(*value == 2);
  psyassert(*loaded == 1);
}

static void test_weak_ptr_shares_control_block() {
  auto owner = std::make_shared<Derived>();
  std::weak_ptr<Derived> derived = owner;
  std::weak_ptr<Base> base = derived;

  psyassert(!base.expired());
  psyassert(base.use_count() == 1);
  {
    auto locked = base.lock();
    psyassert(locked.get() == owner.get());
    psyassert(owner.use_count() == 2);
  }

  owner.reset();
  psyassert(base.expired());
  psyassert(!base.lock());
}

struct VirtualBase {
  virtual ~VirtualBase() = default;
};
struct VirtualDerived : virtual VirtualBase {};

class PrivateSharedFromThis
    : private std::enable_shared_from_this<PrivateSharedFromThis> {};

struct SharedFromThisBase : std::enable_shared_from_this<SharedFromThisBase> {
  virtual ~SharedFromThisBase() = default;
};
struct SharedFromThisDerived : virtual SharedFromThisBase {};

static void test_inaccessible_enable_shared_from_this() {
  auto value = std::make_shared<PrivateSharedFromThis>();
  psyassert(value != nullptr);
}

static void test_inherited_enable_shared_from_this() {
  auto derived = std::make_shared<SharedFromThisDerived>();
  auto base = derived->shared_from_this();
  psyassert(base.get() == derived.get());
  psyassert(base.use_count() == 2);
}

static void test_expired_converting_weak_ptr() {
  std::weak_ptr<VirtualDerived> derived;
  {
    auto owner = std::make_shared<VirtualDerived>();
    derived = owner;
  }
  std::weak_ptr<VirtualBase> base = derived;
  psyassert(base.expired());
}

// Bug: at -O2 the old converting constructor used reinterpret_cast to read
// ctrl_ from a shared_ptr<U>, which is strict-aliasing UB.  GCC exploited
// the UB such that the control block was freed before m_ref could share it,
// leaving m_ref holding a dangling ctrl_ pointer.  The pattern that triggers
// this is a struct with a shared_ptr<Base> member initialised from a
// make_shared<Derived>() prvalue inside a templated constructor body.
struct OwnerBase {
  virtual ~OwnerBase() = default;
};
template <typename T> struct OwnerValue : OwnerBase {
  T& ref;
  explicit OwnerValue(T& r) : ref(r) {}
};
template <typename DerivedT> class Container {
protected:
  std::shared_ptr<OwnerBase> m_owner;

public:
  template <typename T>
  Container(T& val) : m_owner(std::make_shared<OwnerValue<T>>(val)) {}
  long use_count() const { return m_owner.use_count(); }
};
class Slot : public Container<Slot> {
public:
  template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, Slot>)
  Slot(T& val) : Container<Slot>(val) {}
};

static void test_member_init_from_template_prvalue() {
  // Initialising a shared_ptr<Base> member from make_shared<Derived>() inside
  // a constructor.  The control block must be shared, not freed.
  std::vector<std::string> data;
  Slot s(data);
  psyassert(s.use_count() == 1);

  // Copy — would ASAN-trap with use-after-free if ctrl_ is dangling.
  Slot s2 = s;
  psyassert(s.use_count() == 2);
  psyassert(s2.use_count() == 2);
}

int main() {
  std::unique_ptr<void, void (*)(void*)> void_unique(nullptr, [](void*) {});
  std::shared_ptr<void> void_shared;
  psyassert(void_unique.get() == nullptr);
  psyassert(void_shared.get() == nullptr);
  std::ostringstream pointer_text;
  psyassert(&(pointer_text << void_unique) == &pointer_text);
  psyassert(!pointer_text.str().empty());

  test_forwarding();
  test_unique_ptr_equality();
  test_allocate_shared();
  test_converting_copy_ctor();
  test_owner_before();
  test_smart_pointer_relations_and_output();
  test_converting_ctor_from_prvalue();
  test_const_pointer_cast();
  test_uninitialized_exception_cleanup();
  test_enable_shared_from_this();
  test_shared_ptr_from_unique_ptr();
  test_shared_ptr_custom_deleter();
  test_shared_ptr_concurrent_destruction();
  test_shared_ptr_atomic_access();
  test_weak_ptr_shares_control_block();
  test_expired_converting_weak_ptr();
  test_inaccessible_enable_shared_from_this();
  test_inherited_enable_shared_from_this();
  test_member_init_from_template_prvalue();
  test_overaligned_allocation();
}
