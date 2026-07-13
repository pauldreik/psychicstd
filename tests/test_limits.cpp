#include "psyassert.h"
#include <limits>

int main() { psyassert(std::numeric_limits<int>::max() > 0); }
