#pragma once

#include <exception>
#include <iostream>
#include <utility>

namespace bmstu
{
template <typename T>
class stack
{
   public:
    stack() : data_(nullptr), size_(0) {}

    bool empty() const noexcept { return size_ == 0; }

    size_t size() const noexcept { return size_; }

    ~stack()
    {
        clear();
        ::operator delete(data_);
    }

    template <typename... Args>
    void emplace(Args&&... args)
    {
        // выделяем память под новый массив
        T* new_data = (T*)::operator new((size_ + 1) * sizeof(T));
        
        // перемещаем старые элементы
        for (size_t i = 0; i < size_; ++i)
        {
            new (new_data + i) T(std::move(data_[i]));
            (data_ + i)->~T();
        }

        // добавляем новый элемент
        new (new_data + size_) T(std::forward<Args>(args)...);
        
        ::operator delete(data_);
        data_ = new_data;
        ++size_;
    }

    // copy semantics (lvalue)
    void push(const T& value)
    {
        // выделяем память под новый массив
        T* new_data = (T*)::operator new((size_ + 1) * sizeof(T));
        
        // перемещаем старые элементы
        for (size_t i = 0; i < size_; ++i)
        {
            new (new_data + i) T(std::move(data_[i]));
            (data_ + i)->~T();
        }

        // добавляем новый элемент
        new (new_data + size_) T(value);
        
        ::operator delete(data_);
        data_ = new_data;
        ++size_;
    }

    // move semantics (rvalue)
    void push(T&& value)
    {
        // выделяем память под новый массив
        T* new_data = (T*)::operator new((size_ + 1) * sizeof(T));
        
        // перемещаем старые элементы
        for (size_t i = 0; i < size_; ++i)
        {
            new (new_data + i) T(std::move(data_[i]));
            (data_ + i)->~T();
        }

        // добавляем новый элемент
        new (new_data + size_) T(std::move(value));
        
        ::operator delete(data_);
        data_ = new_data;
        ++size_;
    }

    void clear() noexcept
    {
        while (size_ > 0)
        {
            --size_;
            (data_ + size_)->~T();
        }
    }

    void pop()
    {
        if (empty())
            throw std::underflow_error("Stack is empty!");
        --size_;
        (data_ + size_)->~T();
    }

    T& top()
    {
        if (empty())
            throw std::underflow_error("Stack is empty!");
        return data_[size_ - 1];
    }

    const T& top() const
    {
        if (empty())
            throw std::underflow_error("Stack is empty!");
        return data_[size_ - 1];
    }

   private:
    T* data_;
    size_t size_;
};
}  // namespace bmstu