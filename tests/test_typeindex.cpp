#include <cassert>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

// A minimal runtime type registry keyed by type_index -- the mechanism
// behind type-erased factories and pub/sub message dispatch.
struct Shape {
  virtual ~Shape() = default;
  virtual const char* name() const = 0;
};
struct Circle : Shape {
  const char* name() const override { return "circle"; }
};
struct Square : Shape {
  const char* name() const override { return "square"; }
};

int main() {
  std::unordered_map<std::type_index, std::unique_ptr<Shape> (*)()> factory;
  factory[std::type_index(typeid(Circle))] = [] {
    return std::unique_ptr<Shape>(new Circle());
  };
  factory[std::type_index(typeid(Square))] = [] {
    return std::unique_ptr<Shape>(new Square());
  };

  auto shape = factory.at(std::type_index(typeid(Circle)))();
  assert(std::string(shape->name()) == "circle");

  std::type_index ci(typeid(Circle));
  std::type_index ci2(typeid(Circle));
  std::type_index sq(typeid(Square));
  assert(ci == ci2);
  assert(ci != sq);
  assert(ci.hash_code() == ci2.hash_code());
}
