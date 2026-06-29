#include <cassert>
#include <memory>
#include <string>
#include <vector>

struct Base {
  virtual ~Base() = default;
  int x = 42;
};
struct Derived : Base {
  int y = 99;
};

static void test_converting_copy_ctor() {
  std::shared_ptr<Derived> d = std::make_shared<Derived>();
  assert(d.use_count() == 1);

  std::shared_ptr<Base> b = d;
  assert(b.use_count() == 2);
  assert(d.use_count() == 2);

  d.reset();
  assert(b.use_count() == 1);
  assert(b->x == 42);
}

static void test_converting_ctor_from_prvalue() {
  std::shared_ptr<Base> b(std::make_shared<Derived>());
  assert(b.use_count() == 1);
  assert(b->x == 42);

  std::shared_ptr<Base> b2 = b;
  assert(b.use_count() == 2);
  b.reset();
  assert(b2.use_count() == 1);
  assert(b2->x == 42);
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
  assert(s.use_count() == 1);

  // Copy — would ASAN-trap with use-after-free if ctrl_ is dangling.
  Slot s2 = s;
  assert(s.use_count() == 2);
  assert(s2.use_count() == 2);
}

int main() {
  test_converting_copy_ctor();
  test_converting_ctor_from_prvalue();
  test_member_init_from_template_prvalue();
}
