#include "rapidjson/document.h"

int main() {
  rapidjson::Document d;
  d.Parse(R"({"hello":"world","answer":42})");
  return d["answer"].GetInt() == 42 ? 0 : 1;
}
