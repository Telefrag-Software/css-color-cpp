#pragma once
#ifndef CSS_COLOR_CONST_STR_HPP
#define CSS_COLOR_CONST_STR_HPP

#include <string_view>
#include "const_math.hpp"
namespace css_colors::details {

	inline constexpr char const_tolower(char ch) {
		if (ch >= 'A' && ch <= 'Z')return ch - 'A' + 'a';
		return ch;
	}
	inline static constexpr bool const_isspace(char ch)noexcept {
		return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\f' || ch == '\v';
	}
	inline static constexpr bool const_isdigit(char ch)noexcept {
		return (ch >= '0' && ch <= '9');
	}
	inline static constexpr bool const_isalpha(char ch)noexcept {
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
	}
	inline static constexpr bool const_isalnum(char ch)noexcept {
		return const_isdigit(ch) || const_isalpha(ch);
	}
	inline static constexpr bool const_ispunct(char ch)noexcept {
		return ch == '#' || ch == '/' || ch == '(' || ch == ')' || ch == ',';	//others are not used.
	}
	inline static constexpr int8_t const_as_hex(char ch)noexcept {
		if (const_isdigit(ch))return ch - '0';
		char lc = const_tolower(ch);
		if (lc >= 'a' && lc <= 'f')return lc - 'a' + 10;
		return -1;
	}

	inline constexpr bool const_iless(const std::string_view& lhs, const std::string_view& rhs) {
		for (size_t i = 0; i < math::min(lhs.length(), rhs.length()); i++) {
			if (const_tolower(lhs[i]) < const_tolower(rhs[i]))return true;
			if (const_tolower(lhs[i]) > const_tolower(rhs[i]))return false;
		}
		if (lhs.length() < rhs.length())return true;
		return false;
	}
	inline constexpr bool const_iequal(const std::string_view& lhs, const std::string_view& rhs)noexcept {
		if (lhs.size() != rhs.size())return false;
		for (size_t i = 0; i < lhs.size(); i++) {
			if (lhs[i] != rhs[i])return false;
		}
		return true;
	}

}
#endif /* CSS_COLOR_CONST_STR_HPP */
