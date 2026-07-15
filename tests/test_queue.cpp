#include "psyassert.h"
#include <queue>

int main() {
  // queue
  std::queue<int> q;
  psyassert(q.empty());
  q.push(1);
  q.push(2);
  q.emplace(3);
  psyassert(q.size() == 3);
  psyassert(q.front() == 1);
  psyassert(q.back() == 3);
  q.pop();
  psyassert(q.front() == 2);

  // priority_queue: max-heap by default
  std::priority_queue<int> pq;
  psyassert(pq.empty());
  pq.push(3);
  pq.push(1);
  pq.emplace(4);
  pq.push(2);
  psyassert(pq.size() == 4);
  psyassert(pq.top() == 4);
  pq.pop();
  psyassert(pq.top() == 3);
  pq.pop();
  psyassert(pq.top() == 2);
  pq.pop();
  psyassert(pq.top() == 1);
  pq.pop();
  psyassert(pq.empty());

  // heapifying constructor + custom comparator (min-heap)
  std::vector<int> v{5, 9, 2, 7};
  std::priority_queue<int, std::vector<int>, std::greater<int>> minpq(
      std::greater<int>(), v);
  psyassert(minpq.top() == 2);
  minpq.pop();
  psyassert(minpq.top() == 5);
  minpq.pop();
  psyassert(minpq.top() == 7);
  minpq.pop();
  psyassert(minpq.top() == 9);
}
