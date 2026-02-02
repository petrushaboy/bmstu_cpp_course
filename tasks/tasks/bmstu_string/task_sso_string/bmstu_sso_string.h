#pragma once

#include <exception>
#include <iostream>
#include <utility>
#include <algorithm>

namespace bmstu
{
template <typename T>
class basic_string;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template <typename T>
class basic_string
{
   private:
	static constexpr size_t SSO_CAPACITY =
		(sizeof(T*) + sizeof(size_t) + sizeof(size_t)) / sizeof(T) - 1;

	struct LongString
	{
		T* ptr;
		size_t size;
		size_t capacity;
	};

	struct ShortString
	{
		T buffer[SSO_CAPACITY + 1];
		unsigned char size;
	};

	union Data
	{
		LongString long_str;
		ShortString short_str;
		
		Data() : short_str{} {}
		~Data() {}
	};

	Data data_;
	bool is_long_;

	bool is_long() const { return is_long_; }

	T* get_ptr() 
	{ 
		return is_long_ ? data_.long_str.ptr : data_.short_str.buffer; 
	}

	const T* get_ptr() const 
	{ 
		return is_long_ ? data_.long_str.ptr : data_.short_str.buffer; 
	}

	size_t get_size() const 
	{ 
		return is_long_ ? data_.long_str.size : static_cast<size_t>(data_.short_str.size); 
	}

	size_t get_capacity() const 
	{ 
		return is_long_ ? data_.long_str.capacity : SSO_CAPACITY; 
	}

	void set_size(size_t size)
	{
		if (is_long_)
		{
			data_.long_str.size = size;
		}
		else
		{
			data_.short_str.size = static_cast<unsigned char>(size);
		}
	}

   public:
	basic_string() : is_long_(false)
	{
		data_.short_str.size = 0;
		data_.short_str.buffer[0] = T(0);
	}

	basic_string(size_t size)
	{
		if (size <= SSO_CAPACITY)
		{
			is_long_ = false;
			data_.short_str.size = static_cast<unsigned char>(size);
			for (size_t i = 0; i < size; ++i)
			{
				data_.short_str.buffer[i] = T(' ');
			}
			data_.short_str.buffer[size] = T(0);
		}
		else
		{
			is_long_ = true;
			data_.long_str.ptr = new T[size + 1];
			data_.long_str.size = size;
			data_.long_str.capacity = size;
			for (size_t i = 0; i < size; ++i)
			{
				data_.long_str.ptr[i] = T(' ');
			}
			data_.long_str.ptr[size] = T(0);
		}
	}

	basic_string(std::initializer_list<T> il)
	{
		size_t size = il.size();
		if (size <= SSO_CAPACITY)
		{
			is_long_ = false;
			data_.short_str.size = static_cast<unsigned char>(size);
			size_t i = 0;
			for (const T& elem : il)
			{
				data_.short_str.buffer[i++] = elem;
			}
			data_.short_str.buffer[size] = T(0);
		}
		else
		{
			is_long_ = true;
			data_.long_str.ptr = new T[size + 1];
			data_.long_str.size = size;
			data_.long_str.capacity = size;
			size_t i = 0;
			for (const T& elem : il)
			{
				data_.long_str.ptr[i++] = elem;
			}
			data_.long_str.ptr[size] = T(0);
		}
	}

	basic_string(const T* c_str)
	{
		if (!c_str)
		{
			is_long_ = false;
			data_.short_str.size = 0;
			data_.short_str.buffer[0] = T(0);
		}
		else
		{
			size_t len = strlen_(c_str);
			if (len <= SSO_CAPACITY)
			{
				is_long_ = false;
				data_.short_str.size = static_cast<unsigned char>(len);
				for (size_t i = 0; i < len; ++i)
				{
					data_.short_str.buffer[i] = c_str[i];
				}
				data_.short_str.buffer[len] = T(0);
			}
			else
			{
				is_long_ = true;
				data_.long_str.ptr = new T[len + 1];
				data_.long_str.size = len;
				data_.long_str.capacity = len;
				for (size_t i = 0; i < len; ++i)
				{
					data_.long_str.ptr[i] = c_str[i];
				}
				data_.long_str.ptr[len] = T(0);
			}
		}
	}

	basic_string(const basic_string& other) : is_long_(other.is_long_)
	{
		if (is_long_)
		{
			data_.long_str.ptr = new T[other.data_.long_str.size + 1];
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.size;
			for (size_t i = 0; i <= other.data_.long_str.size; ++i)
			{
				data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
			}
		}
		else
		{
			data_.short_str = other.data_.short_str;
		}
	}

	basic_string(basic_string&& other) noexcept : is_long_(other.is_long_)
	{
		if (is_long_)
		{
			data_.long_str = other.data_.long_str;
			other.data_.long_str.ptr = nullptr;
			other.data_.long_str.size = 0;
			other.data_.long_str.capacity = 0;
			other.is_long_ = false;
		}
		else
		{
			data_.short_str = other.data_.short_str;
		}
		other.data_.short_str.size = 0;
		other.data_.short_str.buffer[0] = T(0);
	}

	~basic_string()
	{
		if (is_long_ && data_.long_str.ptr)
		{
			delete[] data_.long_str.ptr;
		}
	}

	const T* c_str() const { return get_ptr(); }

	size_t size() const { return get_size(); }

	bool is_using_sso() const { return !is_long_; }

	size_t capacity() const { return get_capacity(); }

	basic_string& operator=(basic_string&& other) noexcept
	{
		if (this != &other)
		{
			clean_();
			is_long_ = other.is_long_;
			if (is_long_)
			{
				data_.long_str = other.data_.long_str;
				other.data_.long_str.ptr = nullptr;
				other.data_.long_str.size = 0;
				other.data_.long_str.capacity = 0;
				other.is_long_ = false;
			}
			else
			{
				data_.short_str = other.data_.short_str;
			}
			other.data_.short_str.size = 0;
			other.data_.short_str.buffer[0] = T(0);
		}
		return *this;
	}

	basic_string& operator=(const T* c_str)
	{
		if (!c_str)
		{
			clean_();
			is_long_ = false;
			data_.short_str.size = 0;
			data_.short_str.buffer[0] = T(0);
			return *this;
		}
		
		size_t len = strlen_(c_str);
		if (len <= SSO_CAPACITY)
		{
			clean_();
			is_long_ = false;
			data_.short_str.size = static_cast<unsigned char>(len);
			for (size_t i = 0; i < len; ++i)
			{
				data_.short_str.buffer[i] = c_str[i];
			}
			data_.short_str.buffer[len] = T(0);
		}
		else
		{
			if (is_long_ && data_.long_str.capacity >= len)
			{
				for (size_t i = 0; i < len; ++i)
				{
					data_.long_str.ptr[i] = c_str[i];
				}
				data_.long_str.ptr[len] = T(0);
				data_.long_str.size = len;
			}
			else
			{
				clean_();
				is_long_ = true;
				data_.long_str.ptr = new T[len + 1];
				data_.long_str.size = len;
				data_.long_str.capacity = len;
				for (size_t i = 0; i < len; ++i)
				{
					data_.long_str.ptr[i] = c_str[i];
				}
				data_.long_str.ptr[len] = T(0);
			}
		}
		return *this;
	}

	basic_string& operator=(const basic_string& other)
	{
		if (this != &other)
		{
			clean_();
			is_long_ = other.is_long_;
			if (is_long_)
			{
				data_.long_str.ptr = new T[other.data_.long_str.size + 1];
				data_.long_str.size = other.data_.long_str.size;
				data_.long_str.capacity = other.data_.long_str.size;
				for (size_t i = 0; i <= other.data_.long_str.size; ++i)
				{
					data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
				}
			}
			else
			{
				data_.short_str = other.data_.short_str;
			}
		}
		return *this;
	}

	friend basic_string<T> operator+(const basic_string<T>& left,
									 const basic_string<T>& right)
	{
		size_t total = left.size() + right.size();
		basic_string<T> result(total);
		
		const T* left_ptr = left.get_ptr();
		const T* right_ptr = right.get_ptr();
		T* result_ptr = result.get_ptr();
		
		for (size_t i = 0; i < left.size(); ++i)
		{
			result_ptr[i] = left_ptr[i];
		}
		
		for (size_t i = 0; i < right.size(); ++i)
		{
			result_ptr[left.size() + i] = right_ptr[i];
		}
		
		result_ptr[total] = T(0);
		result.set_size(total);
		
		return result;
	}

	template <typename S>
	friend S& operator<<(S& os, const basic_string& obj)
	{
		const T* ptr = obj.get_ptr();
		for (size_t i = 0; i < obj.size(); ++i)
		{
			os << ptr[i];
		}
		return os;
	}

	template <typename S>
	friend S& operator>>(S& is, basic_string& obj)
	{
		obj.clean_();
		obj.is_long_ = false;
		obj.data_.short_str.size = 0;
		obj.data_.short_str.buffer[0] = T(0);
		
		T ch;
		while (is.get(ch) && ch != '\n' && ch != ' ' && ch != '\t')
		{
			obj += ch;
		}
		
		if (ch == '\n' || ch == ' ' || ch == '\t')
		{
			is.putback(ch);
		}
		
		return is;
	}

	basic_string& operator+=(const basic_string& other)
	{
		size_t old_len = size();
		size_t new_len = old_len + other.size();
		
		if (new_len <= get_capacity())
		{
			T* ptr = get_ptr();
			const T* other_ptr = other.get_ptr();
			
			for (size_t i = 0; i < other.size(); ++i)
			{
				ptr[old_len + i] = other_ptr[i];
			}
			ptr[new_len] = T(0);
			set_size(new_len);
		}
		else
		{
			size_t new_cap = std::max(new_len, get_capacity() * 2);
			T* new_ptr = new T[new_cap + 1];
			
			const T* old_ptr = get_ptr();
			for (size_t i = 0; i < old_len; ++i)
			{
				new_ptr[i] = old_ptr[i];
			}
			
			const T* other_ptr = other.get_ptr();
			for (size_t i = 0; i < other.size(); ++i)
			{
				new_ptr[old_len + i] = other_ptr[i];
			}
			
			new_ptr[new_len] = T(0);
			
			clean_();
			
			is_long_ = true;
			data_.long_str.ptr = new_ptr;
			data_.long_str.size = new_len;
			data_.long_str.capacity = new_cap;
		}
		
		return *this;
	}

	basic_string& operator+=(T symbol)
	{
		size_t old_len = size();
		size_t new_len = old_len + 1;
		
		if (new_len <= get_capacity())
		{
			T* ptr = get_ptr();
			ptr[old_len] = symbol;
			ptr[new_len] = T(0);
			set_size(new_len);
		}
		else
		{
			size_t new_cap = std::max(new_len, get_capacity() * 2);
			T* new_ptr = new T[new_cap + 1];
			
			const T* old_ptr = get_ptr();
			for (size_t i = 0; i < old_len; ++i)
			{
				new_ptr[i] = old_ptr[i];
			}
			
			new_ptr[old_len] = symbol;
			new_ptr[new_len] = T(0);
			
			clean_();
			
			is_long_ = true;
			data_.long_str.ptr = new_ptr;
			data_.long_str.size = new_len;
			data_.long_str.capacity = new_cap;
		}
		
		return *this;
	}

	T& operator[](size_t index) noexcept
	{
		return get_ptr()[index];
	}

	T& at(size_t index)
	{
		if (index >= size())
		{
			throw std::out_of_range("Invalid index");
		}
		return get_ptr()[index];
	}

	T* data()
	{
		return get_ptr();
	}

   private:
	static size_t strlen_(const T* str)
	{
		const T* p = str;
		while (*p)
			++p;
		return p - str;
	}

	void clean_()
	{
		if (is_long_ && data_.long_str.ptr)
		{
			delete[] data_.long_str.ptr;
			data_.long_str.ptr = nullptr;
			data_.long_str.size = 0;
			data_.long_str.capacity = 0;
		}
	}
};
}  // namespace bmstu