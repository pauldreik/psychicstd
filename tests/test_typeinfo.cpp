#include <cassert>
#include <typeinfo>

int main() { assert(typeid(int) == typeid(int)); }
