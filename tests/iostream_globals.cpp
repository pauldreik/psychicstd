#include <iostream>

namespace {

bool cout_was_ready = std::cout.rdbuf() != nullptr;

} // namespace

bool iostream_was_ready_during_static_initialization() {
  return cout_was_ready;
}

std::ostream* iostream_cout_from_other_translation_unit() { return &std::cout; }
