#pragma once

#include <iostream>
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <iterator>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(const size_t size)
        : reserve_(size)
    {}

    size_t GetReserveSize() const {
        return reserve_;
    }
private:
    size_t reserve_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(begin(), end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())
        , size_(init.size())
        , capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const ReserveProxyObj& proxy) {
        Reserve(proxy.GetReserveSize());
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other)
        : items_(std::move(other.items_))
        , size_(std::exchange(other.size_, 0))
        , capacity_(std::exchange(other.capacity_, 0))
    {}

    // Оператор присваивания перемещения
    SimpleVector& operator=(SimpleVector&& other) {
        items_ = std::move(other.items_);
        size_  = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
        {
            using namespace std::literals;
            throw std::out_of_range("Index out of range"s);
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
        {
            using namespace std::literals;
            throw std::out_of_range("Index out of range"s);
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t old_size = size_;
            ArrayPtr<Type> temp_items(new_size);
            items_.swap(temp_items);
            size_ = new_size;
            capacity_ = new_size;

            std::copy(std::make_move_iterator(temp_items.Get()), std::make_move_iterator(temp_items.Get() + old_size), begin());
            std::generate(begin() + old_size, end(), []() { return Type{}; });
        }
        else if (new_size >= size_) {
            size_t old_size = size_;
            size_ = new_size;

            std::generate(begin() + old_size, end(), []() { return Type{}; });
        }
        else {
            size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

    // Конструктор копирования
    SimpleVector(const SimpleVector& other)
        : items_(other.GetSize())
        , size_(other.GetSize())
        , capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), begin());
    }

    // Оператор присваивания
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp_copy(rhs);
            this->swap(tmp_copy);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ + 1 > capacity_) {
            size_t new_size = size_ == 0 ? 1 : size_ + 1;
            size_t new_capacity = size_ == 0 ? new_size : size_ * 2;

            ArrayPtr<Type> temp_items(new_capacity);
            items_.swap(temp_items);
            capacity_ = new_capacity;
            size_ = new_size;

            std::copy(temp_items.Get(), temp_items.Get() + size_ - 1, begin());
            items_[size_ - 1] = item;
        }
        else {
            items_[size_++] = item;
        }
    }

    // Добавляет элемент в конец вектора, используя перемещение
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type&& item) {
        if (size_ + 1 > capacity_) {
            size_t new_size = size_ == 0 ? 1 : size_ + 1;
            size_t new_capacity = size_ == 0 ? new_size : size_ * 2;

            ArrayPtr<Type> temp_items(new_capacity);
            items_.swap(temp_items);
            capacity_ = new_capacity;
            size_ = new_size;

            std::copy(std::make_move_iterator(temp_items.Get()), std::make_move_iterator(temp_items.Get() + size_ - 1), begin());
            items_[size_ - 1] = std::move(item);
        }
        else {
            items_[size_++] = std::move(item);
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора увеличивается вдвое, а для вектора вместимостью 0 становится равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = pos - begin();
        assert(index <= size_);
        if (size_ + 1 > capacity_) {
            size_t new_size = size_ == 0 ? 1 : size_ + 1;
            size_t new_capacity = size_ == 0 ? new_size : size_ * 2;

            ArrayPtr<Type> temp_items(new_capacity);
            items_.swap(temp_items);
            capacity_ = new_capacity;
            size_ = new_size;

            std::copy(temp_items.Get(), temp_items.Get() + index, begin());
            items_[index] = value;
            std::copy(temp_items.Get() + index, temp_items.Get() + size_ - 1, begin() + index + 1);
        }
        else {
            size_++;
            std::copy_backward(items_.Get() + index, items_.Get() + size_ - 1, items_.Get() + size_);
            items_[index] = value;
        }
        return begin() + index;
    }

    // Вставляет значение value в позицию pos, используя перемещение
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора увеличивается вдвое, а для вектора вместимостью 0 становится равной 1
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t index = pos - begin();
        assert(index <= size_);
        if (size_ + 1 > capacity_) {
            size_t new_size = size_ == 0 ? 1 : size_ + 1;
            size_t new_capacity = size_ == 0 ? new_size : size_ * 2;

            ArrayPtr<Type> temp_items(new_capacity);
            items_.swap(temp_items);
            capacity_ = new_capacity;
            size_ = new_size;

            std::copy(std::make_move_iterator(temp_items.Get()), std::make_move_iterator(temp_items.Get() + index), begin());
            items_[index] = std::move(value);
            std::copy(std::make_move_iterator(temp_items.Get() + index), std::make_move_iterator(temp_items.Get() + size_ - 1), begin() + index + 1);
        }
        else {
            size_++;
            std::copy_backward(std::make_move_iterator(items_.Get() + index), std::make_move_iterator(items_.Get() + size_ - 1), items_.Get() + size_);
            items_[index] = std::move(value);
        }
        return begin() + index;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ != 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t index = pos - begin();
        assert(index < size_);
        if (size_) {
            std::copy(std::make_move_iterator(begin() + index + 1), std::make_move_iterator(end()), begin() + index);
            size_--;
        }
        return begin() + index;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_ , other.size_);
        std::swap(capacity_ , other.capacity_);
    }

    // "Резервирует" память для хранения capacity элементов
    void Reserve(const size_t capacity) {
        if (capacity > capacity_) {
            ArrayPtr<Type> temp_items(capacity);
            items_.swap(temp_items);
            capacity_ = capacity;

            std::copy(std::make_move_iterator(temp_items.Get()), std::make_move_iterator(temp_items.Get()) + size_, begin());
        }
    }

private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (rhs < lhs);
}

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

