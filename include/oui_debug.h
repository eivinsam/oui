#pragma once

#include <string>
#include <charconv>

namespace oui::debug
{
	template <class T>
	struct hex
	{
		static_assert(std::is_integral_v<T>);

		T value;

		constexpr hex(T value) : value(value) { }
	};

	void print(const char* text);

	inline void print(const std::string& text) { print(text.c_str()); }

	template <class T>
	std::enable_if_t<std::is_integral_v<T>> 
		print(T number)
	{ 
		char buffer[16];
		const auto result = std::to_chars(std::begin(buffer), std::end(buffer)-1, number, 10);
		*result.ptr = 0;
		print(buffer); 
	}
	template <class T>
	void print(hex<T> number)
	{
		char buffer[16] = { '0', 'x' };
		const auto result = std::to_chars(std::begin(buffer)+2, std::end(buffer) - 1, number.value, 16);
		*result.ptr = 0;
		print(buffer);
	}

	template <class T>
	std::enable_if_t<std::is_floating_point_v<T>>
		print(T number) { print(std::to_string(number)); }



	template <class First, class Second, class... Rest>
	void print(First&& first, Second&& second, Rest&&... rest) 
	{
		print(std::forward<First>(first));
		print(std::forward<Second>(second));
		(print(std::forward<Rest>(rest)), ...);
	}

	template <class... Args>
	void println(Args&&... args)
	{
		print(std::forward<Args>(args)..., "\n");
	}
}
