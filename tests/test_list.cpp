#include "psyassert.h"
#include <list>
#include <utility>

#ifdef PSYCHICSTD_TEST_PSYCHICSTD
static_assert(sizeof(std::list<int>::iterator) == sizeof(void*));
#endif

struct sortable_item {
  int key;
  int order = 0;

  bool operator<(const sortable_item& other) const { return key < other.key; }
};

template <typename T> struct stateful_allocator {
  using value_type = T;

  int state;

  explicit stateful_allocator(int state) : state(state) {}

  template <typename U>
  stateful_allocator(const stateful_allocator<U>& other) : state(other.state) {}

  T* allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }
  void deallocate(T* p, std::size_t n) { std::allocator<T>{}.deallocate(p, n); }

  template <typename U>
  bool operator==(const stateful_allocator<U>& other) const {
    return state == other.state;
  }
};

int main() {
  std::list<int> l = {1, 2, 3};
  psyassert(l.front() == 1);
  std::list<int>::const_iterator const_begin = l.begin();
  psyassert(l.begin() == const_begin);
  psyassert(const_begin == l.begin());

  std::allocator<int> a;
  const std::list<int> la(a);
  psyassert(la.get_allocator() == a);

  stateful_allocator<int> ta(1);
  const std::list<int, stateful_allocator<int>> tl(ta);
  psyassert(tl.get_allocator() == ta);

  std::list<int> stable_end;
  auto old_end = stable_end.end();
  stable_end.push_back(7);
  psyassert(*--old_end == 7);

  std::list<int> resized = {1, 2};
  resized.resize(4, 7);
  psyassert((resized == std::list<int>{1, 2, 7, 7}));
  resized.resize(5);
  psyassert(resized.back() == 0);
  resized.resize(1);
  psyassert((resized == std::list<int>{1}));

  std::list<int> recycled = {8};
  std::list<int> reactor_queue;
  auto queue_end = reactor_queue.end();
  reactor_queue.splice(queue_end, recycled, recycled.begin());
  psyassert(*--queue_end == 8);

  std::list<int> source = {4, 5};
  l.splice(l.end(), source);
  psyassert(source.empty());
  psyassert(l.size() == 5);
  psyassert(l.back() == 5);

  std::list<int> middle = {1, 4};
  std::list<int> inserted = {2, 3};
  int* inserted_first = &inserted.front();
  int* inserted_last = &inserted.back();
  middle.splice(++middle.begin(), inserted);
  psyassert(inserted.empty());
  psyassert((middle == std::list<int>{1, 2, 3, 4}));
  psyassert(&*++middle.begin() == inserted_first);
  psyassert(&*std::next(middle.begin(), 2) == inserted_last);

  std::list<int> splice_source = {2, 3};
  std::list<int> splice_destination = {1, 4};
  auto transferred = splice_source.begin();
  auto insertion_point = ++splice_destination.begin();
  splice_destination.splice(insertion_point, splice_source, transferred);
  psyassert(*transferred == 2);
  auto before_transferred = transferred;
  psyassert(*--before_transferred == 1);
  auto after_transferred = transferred;
  psyassert(++after_transferred == insertion_point);
  psyassert(*after_transferred == 4);

  std::list<int> range_source = {2, 3, 6};
  std::list<int> range_destination = {1, 4, 5};
  auto range_first = range_source.begin();
  auto range_last = ++range_source.begin();
  auto range_end = std::next(range_source.begin(), 2);
  auto range_insertion_point = ++range_destination.begin();
  range_destination.splice(range_insertion_point, range_source, range_first,
                           range_end);
  auto before_range = range_first;
  psyassert(*--before_range == 1);
  psyassert(*range_first == 2);
  psyassert(*range_last == 3);
  psyassert(++range_last == range_insertion_point);
  psyassert(*range_last == 4);
  psyassert((range_destination == std::list<int>{1, 2, 3, 4, 5}));
  psyassert((range_source == std::list<int>{6}));

  std::list<int> whole_source = {5, 6};
  std::list<int> whole_destination = {4, 7};
  auto whole_first = whole_source.begin();
  auto whole_last = ++whole_source.begin();
  whole_destination.splice(++whole_destination.begin(), whole_source);
  auto before_whole = whole_first;
  psyassert(*--before_whole == 4);
  psyassert(*whole_first == 5);
  psyassert(*whole_last == 6);
  psyassert(*++whole_last == 7);

  std::list<int> move_source = {8, 9};
  auto moved_iterator = ++move_source.begin();
  std::list<int> move_destination(std::move(move_source));
  psyassert(*moved_iterator == 9);
  psyassert(++moved_iterator == move_destination.end());

  std::list<int> assigned_source = {10, 11};
  auto assigned_iterator = assigned_source.begin();
  std::list<int> assigned_destination = {12};
  assigned_destination = std::move(assigned_source);
  psyassert(*assigned_iterator == 10);
  psyassert(++assigned_iterator != assigned_destination.end());
  psyassert(*assigned_iterator == 11);
  psyassert(++assigned_iterator == assigned_destination.end());

  auto reverse = l.rbegin();
  psyassert(*reverse++ == 5);
  psyassert(*reverse == 4);

  std::list<sortable_item> sortable = {{3}, {1}, {2}};
  sortable_item* first = &sortable.front();
  sortable_item* second = &*++sortable.begin();
  sortable_item* third = &*std::next(sortable.begin(), 2);
  sortable.sort();
  psyassert(&sortable.front() == second);
  psyassert(&*++sortable.begin() == third);
  psyassert(&sortable.back() == first);

  std::list<sortable_item> stable = {{2, 0}, {1, 1}, {2, 2}, {1, 3}};
  stable.sort();
  auto stable_it = stable.begin();
  psyassert(stable_it++->order == 1);
  psyassert(stable_it++->order == 3);
  psyassert(stable_it++->order == 0);
  psyassert(stable_it++->order == 2);

  stable.sort([](const sortable_item& a, const sortable_item& b) {
    return a.key > b.key;
  });
  psyassert(stable.front().key == 2);
  psyassert(stable.back().key == 1);

  std::list<int> many;
  for (int i = 0; i < 1024; ++i)
    many.push_back(i);
  int comparisons = 0;
  many.sort([&](int a, int b) {
    ++comparisons;
    return a < b;
  });
  psyassert(comparisons < 20 * 1024);

  many.clear();
  for (int i = 0; i < 1000; ++i)
    many.push_back((i * 37) % 1000);
  many.sort();
  int expected = 0;
  for (int item : many)
    psyassert(item == expected++);

  int throws_after = 10;
  bool threw = false;
  try {
    many.sort([&](int a, int b) {
      if (--throws_after == 0)
        throw 1;
      return a > b;
    });
  } catch (int) {
    threw = true;
  }
  psyassert(threw);
  std::size_t forward_count = 0;
  for (auto it = many.begin(); it != many.end(); ++it)
    ++forward_count;
  std::size_t reverse_count = 0;
  for (auto it = many.rbegin(); it != many.rend(); ++it)
    ++reverse_count;
  psyassert(forward_count == many.size());
  psyassert(reverse_count == many.size());
}
