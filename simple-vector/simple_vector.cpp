#include "simple_vector.h"

void PrintSimpleVector(SimpleVector<int> v) {
    using namespace std::literals;
    for (auto& e : v) {
        std::cout << e << " "s;
    }
    std::cout << std::endl;
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
