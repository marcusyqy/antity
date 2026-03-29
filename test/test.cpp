#include <stdio.h>
#include "../antity/graph/graph.hpp"

#if 0
int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  ant::Graph graph{};
  auto x = graph.storage<int>(5);
  auto y = graph.storage<float>(5.f);
  auto op = graph.operation([](int, float) {});

  auto [z, v] = op(x, y);

  fprintf(stdout, "Hello world\n");
  return 0;
}

#else

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  fprintf(stdout, "hello world\n");
  return 0;
}
#endif