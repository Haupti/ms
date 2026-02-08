#include "../lib/asap/t.hpp"

void test_example(T* t) {
    t->assert(true);
}

int main() {
    T t("ExampleSuite");
    t.test("SimpleAssert", test_example);
    return 0;
}
