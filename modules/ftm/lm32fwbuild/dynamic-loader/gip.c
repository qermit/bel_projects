static int foo; // global w/o static = DOOM

static int bar(int x) { // all functions you plan to call must be static
  return x + foo;
}

static void set(int x) {
  foo = x;
}

__attribute__((visibility("hidden"))) // this makes the entry function not subject to symbol overloads
int entry(int k) {
  set(6);
  return bar(5);
}
