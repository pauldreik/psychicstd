#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <cassert>
#include <cstring>

int main() {
  using namespace rapidjson;

  // Round-trip: parse then write back
  {
    const char* input = R"({"answer":42,"ok":true})";
    Document d;
    d.Parse(input);
    assert(!d.HasParseError());

    StringBuffer buf;
    Writer<StringBuffer> writer(buf);
    d.Accept(writer);
    assert(strcmp(buf.GetString(), input) == 0);
  }

  // Build a document programmatically and write it
  {
    Document d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    d.AddMember("name", Value("psychicstd", alloc), alloc);
    d.AddMember("version", Value(1), alloc);

    Value tags(kArrayType);
    tags.PushBack(Value("fast", alloc), alloc);
    tags.PushBack(Value("lean", alloc), alloc);
    d.AddMember("tags", tags, alloc);

    StringBuffer buf;
    Writer<StringBuffer> writer(buf);
    d.Accept(writer);

    // Re-parse the output and verify
    Document check;
    check.Parse(buf.GetString());
    assert(!check.HasParseError());
    assert(check["name"] == "psychicstd");
    assert(check["version"].GetInt() == 1);
    assert(check["tags"].Size() == 2);
    assert(check["tags"][0] == "fast");
    assert(check["tags"][1] == "lean");
  }

  // PrettyWriter produces valid JSON
  {
    Document d;
    d.Parse(R"({"a":1,"b":[2,3]})");
    assert(!d.HasParseError());

    StringBuffer buf;
    PrettyWriter<StringBuffer> writer(buf);
    d.Accept(writer);

    Document round;
    round.Parse(buf.GetString());
    assert(!round.HasParseError());
    assert(round["a"].GetInt() == 1);
    assert(round["b"][0].GetInt() == 2);
    assert(round["b"][1].GetInt() == 3);
  }

  // Nested write
  {
    Document d;
    d.SetObject();
    auto& alloc = d.GetAllocator();

    Value inner(kObjectType);
    inner.AddMember("x", 10, alloc);
    inner.AddMember("y", 20, alloc);
    d.AddMember("point", inner, alloc);

    StringBuffer buf;
    Writer<StringBuffer> writer(buf);
    d.Accept(writer);

    Document check;
    check.Parse(buf.GetString());
    assert(!check.HasParseError());
    assert(check["point"]["x"].GetInt() == 10);
    assert(check["point"]["y"].GetInt() == 20);
  }

  return 0;
}
