#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <cassert>

int main() {
  using namespace rapidjson;

  // Add and remove members
  {
    Document d;
    d.Parse(R"({"a":1,"b":2,"c":3})");
    assert(!d.HasParseError());

    auto& alloc = d.GetAllocator();
    d.AddMember("d", 4, alloc);
    assert(d.MemberCount() == 4);
    assert(d["d"].GetInt() == 4);

    d.RemoveMember("b");
    assert(d.MemberCount() == 3);
    assert(!d.HasMember("b"));
    assert(d.HasMember("a"));
    assert(d.HasMember("c"));
    assert(d.HasMember("d"));
  }

  // Modify values in place
  {
    Document d;
    d.Parse(R"({"count":0,"label":"old"})");
    assert(!d.HasParseError());

    auto& alloc = d.GetAllocator();
    d["count"].SetInt(99);
    d["label"].SetString("new", alloc);

    assert(d["count"].GetInt() == 99);
    assert(d["label"] == "new");
  }

  // Array manipulation
  {
    Document d;
    d.Parse(R"({"items":[1,2,3,4,5]})");
    assert(!d.HasParseError());

    auto& alloc = d.GetAllocator();
    auto& arr = d["items"];
    arr.PushBack(6, alloc);
    assert(arr.Size() == 6);
    assert(arr[5].GetInt() == 6);

    arr.Erase(arr.Begin());
    assert(arr.Size() == 5);
    assert(arr[0].GetInt() == 2);
  }

  // Deep copy
  {
    Document src;
    src.Parse(R"({"nested":{"x":1}})");
    assert(!src.HasParseError());

    Document dst;
    dst.CopyFrom(src, dst.GetAllocator());
    assert(!dst.HasParseError());
    assert(dst["nested"]["x"].GetInt() == 1);

    // Mutate original, copy is independent
    src["nested"]["x"].SetInt(99);
    assert(dst["nested"]["x"].GetInt() == 1);
  }

  // JSON Pointer
  {
    Document d;
    d.Parse(R"({"a":{"b":{"c":42}},"arr":[10,20,30]})");
    assert(!d.HasParseError());

    assert(Pointer("/a/b/c").Get(d)->GetInt() == 42);
    assert(Pointer("/arr/1").Get(d)->GetInt() == 20);

    Pointer("/a/b/c").Set(d, 100);
    assert(d["a"]["b"]["c"].GetInt() == 100);
  }

  // Member iteration
  {
    Document d;
    d.Parse(R"({"x":1,"y":2,"z":3})");
    assert(!d.HasParseError());

    int sum = 0;
    for (auto it = d.MemberBegin(); it != d.MemberEnd(); ++it)
      sum += it->value.GetInt();
    assert(sum == 6);
  }

  // String with special characters
  {
    Document d;
    d.Parse(R"({"msg":"hello\nworld\t!"})");
    assert(!d.HasParseError());
    // The string contains actual newline and tab after unescaping
    const char* s = d["msg"].GetString();
    assert(s[5] == '\n');
    assert(s[11] == '\t');
  }

  return 0;
}
