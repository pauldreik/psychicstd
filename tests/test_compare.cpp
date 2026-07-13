#include "psyassert.h"
#include <compare>

int main() { psyassert(std::strong_ordering::equal == 0); }
