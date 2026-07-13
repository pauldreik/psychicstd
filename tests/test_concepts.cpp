#include "psyassert.h"
#include <concepts>

int main() { static_assert(std::same_as<int, int>); }
