#include <cassert>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

// Dijkstra-style usage: a priority_queue of (distance, node) pairs, always
// popping the closest unvisited node -- the textbook reason priority_queue
// exists.
int main() {
  using Edge = std::pair<int, int>; // (distance, node)
  std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;

  pq.push({0, 0});
  pq.push({4, 1});
  pq.push({1, 2});
  pq.push({2, 3});

  std::vector<int> pop_order;
  while (!pq.empty()) {
    pop_order.push_back(pq.top().second);
    pq.pop();
  }
  // Nodes must come out sorted by distance: 0(d0), 2(d1), 3(d2), 1(d4).
  assert((pop_order == std::vector<int>{0, 2, 3, 1}));

  std::queue<int> q;
  q.push(1);
  q.push(2);
  q.push(3);
  assert(q.front() == 1 && q.back() == 3);
  q.pop();
  assert(q.front() == 2);
  assert(q.size() == 2);
}
