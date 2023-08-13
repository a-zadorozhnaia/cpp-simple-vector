#include "simple_vector.h"

int main() {
    // Создание и заполнение вектора
    SimpleVector<int> example_vector;
    size_t size = 10;
    for (size_t i = 0; i < size; ++i) {
        example_vector.PushBack(i);
    }
    PrintSimpleVector(example_vector);

    // Вставка элементов
    example_vector.Insert(example_vector.begin(), 13);
    example_vector.Insert(example_vector.end(), 14);
    example_vector.Insert(example_vector.begin() + 3, 15);
    PrintSimpleVector(example_vector);

    // Удаление элементов
    example_vector.Erase(example_vector.begin());
    PrintSimpleVector(example_vector);

    // Изменение элементов
    for (size_t i = 0; i < example_vector.GetSize(); i++) {
        example_vector[i] += 1;
    }
    PrintSimpleVector(example_vector);

    return 0;
}
