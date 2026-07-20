#include "psyassert.h"
#include <memory>
#include <string>
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
}

struct SelfShared : std::enable_shared_from_this<SelfShared> {
  std::shared_ptr<SelfShared> self() { return shared_from_this(); }
};

static void test_enable_shared_from_this() {
  auto owner = std::make_shared<SelfShared>();
  auto self = owner->self();
  psyassert(self.get() == owner.get());
  psyassert(owner.use_count() == 2);
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

  test_forwarding();
  test_allocate_shared();
  test_converting_copy_ctor();
  test_converting_ctor_from_prvalue();
  test_const_pointer_cast();
  test_enable_shared_from_this();
  test_shared_ptr_from_unique_ptr();
  test_shared_ptr_custom_deleter();
  test_member_init_from_template_prvalue();
}
