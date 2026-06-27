#include "rapidjson/document.h"
#include <cassert>

int main() {
  using namespace rapidjson;

  // Basic object
  Document d;
  d.Parse(R"({
        "name": "psychicstd",
        "version": 1,
        "ratio": 2.4,
        "active": true,
        "nothing": null,
        "tags": ["fast", "cpp", "headers"]
    })");
  assert(!d.HasParseError());
  assert(d.IsObject());

  assert(d["name"].IsString());
  assert(d["name"] == "psychicstd");

  assert(d["version"].IsInt());
  assert(d["version"].GetInt() == 1);

  assert(d["ratio"].IsDouble());
  assert(d["ratio"].GetDouble() > 2.0);

  assert(d["active"].IsBool());
  assert(d["active"].GetBool() == true);

  assert(d["nothing"].IsNull());

  assert(d["tags"].IsArray());
  assert(d["tags"].Size() == 3);
  assert(d["tags"][0] == "fast");
  assert(d["tags"][1] == "cpp");
  assert(d["tags"][2] == "headers");

  // Nested objects
  Document nested;
  nested.Parse(R"({"a":{"b":{"c":42}}})");
  assert(!nested.HasParseError());
  assert(nested["a"]["b"]["c"].GetInt() == 42);

  // Array of objects
  Document arr;
  arr.Parse(R"([{"x":1},{"x":2},{"x":3}])");
  assert(!arr.HasParseError());
  assert(arr.IsArray());
  assert(arr.Size() == 3);
  int sum = 0;
  for (auto& v : arr.GetArray())
    sum += v["x"].GetInt();
  assert(sum == 6);

  // Parse error
  Document bad;
  bad.Parse("{invalid}");
  assert(bad.HasParseError());

  // Integer boundaries
  Document nums;
  nums.Parse(
      R"({"big":2147483647,"neg":-2147483648,"u64":18446744073709551615})");
  assert(!nums.HasParseError());
  assert(nums["big"].GetInt() == 2147483647);
  assert(nums["neg"].GetInt() == -2147483648);
  assert(nums["u64"].GetUint64() == 18446744073709551615ULL);

  // Empty object and array
  Document empty_obj;
  empty_obj.Parse("{}");
  assert(!empty_obj.HasParseError());
  assert(empty_obj.MemberCount() == 0);

  Document empty_arr;
  empty_arr.Parse("[]");
  assert(!empty_arr.HasParseError());
  assert(empty_arr.Size() == 0);

  return 0;
}
