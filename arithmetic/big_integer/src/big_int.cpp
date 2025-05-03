//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"

#include <algorithm>
#include <cmath>
#include <compare>
#include <exception>
#include <ranges>
#include <sstream>
#include <string>

using ull = unsigned long long;

void big_int::normalize()
{
	while (_digits.size() > 1 && _digits.back() == 0)
	{
		_digits.pop_back();
	}
	if (_digits.size() == 1 && _digits[0] == 0)
	{
		_sign = true;
	}
}

bool validate_str(const std::string &str, int radix)
{
	if (str.empty())
		return false;

	size_t start_idx = (str[0] == '-') ? 1 : 0;
	if (start_idx == str.size())
		return false;

	for (auto c : str)
	{
		c = std::toupper(c);
		if (std::isdigit(c) && (c - '0') >= radix)
		{
			return false;
		}
		else if (c >= 'A' && c <= 'Z' && c - 'A' + 10 >= radix)
		{
			return false;
		}
	}

	return true;
}

std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept
{
	if (_sign != other._sign)
	{
		return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
	}

	if (_digits.size() != other._digits.size())
	{
		if (_digits.size() < other._digits.size())
		{
			return _sign ? std::strong_ordering::less : std::strong_ordering::greater;
		}
		else
		{
			return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
		}
	}

	for (size_t i = _digits.size(); i-- > 0;)
	{
		if (_digits[i] != other._digits[i])
		{
			if (_digits[i] < other._digits[i])
			{
				return _sign ? std::strong_ordering::less : std::strong_ordering::greater;
			}
			else
			{
				return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
			}
		}
	}

	return std::strong_ordering::equal;
}

big_int::operator bool() const noexcept { return !(_digits.size() == 1 && _digits[0] == 0); }

big_int &big_int::operator++() &
{
	*this += big_int(1);
	return *this;
}

big_int big_int::operator++(int)
{
	big_int temp(*this);
	++(*this);
	return temp;
}

big_int &big_int::operator--() &
{
	*this -= big_int(1);
	return *this;
}

big_int big_int::operator--(int)
{
	big_int temp(*this);
	--(*this);
	return temp;
}

big_int &big_int::operator+=(const big_int &other) &
{
	*this = *this + other;
	return *this;
}

big_int big_int::abs() const
{
	big_int result(*this);
	result._sign = true;
	return result;
}

big_int &big_int::operator-=(const big_int &other) &
{
	big_int temp = other;
	temp._sign = !temp._sign;
	return *this += temp;
}

big_int big_int::operator+(const big_int &other) const
{
	big_int result(*this);
	if (result._sign == other._sign)
	{
		const size_t n = std::max(result._digits.size(), other._digits.size());
		result._digits.resize(n, 0);

		const ull BASE = ull(1) << sizeof(unsigned int) * 8;
		ull carry = 0;

		for (size_t i = 0; i < n; ++i)
		{
			ull l = static_cast<ull>(result._digits[i]);
			ull r = (i < other._digits.size()) ? static_cast<ull>(other._digits[i]) : 0;
			ull sum = l + r + carry;
			result._digits[i] = static_cast<unsigned int>(sum % BASE);
			carry = sum / BASE;
		}
		if (carry != 0)
		{
			result._digits.push_back(static_cast<unsigned int>(carry));
		}
	}
	else
	{
		if (result.abs() >= other.abs())
		{
			return result - other;
		}
		else
		{
			return other - result;
		}
	}
	result.normalize();
	return result;
}

big_int big_int::operator-(const big_int& other) const {
    big_int result(*this);
	ull BASE = static_cast<ull>(1) << (sizeof(unsigned int)*8);

    if (result._sign != other._sign) {
		big_int temp = result.abs() + other.abs();
		temp._sign = result._sign;
		return temp;
    }
    else {
        bool this_bigger = (result.abs() >= other.abs());

        if (this_bigger) {
            ull borrow = 0;
            const size_t n = result._digits.size();
            for (size_t i = 0; i < n; ++i) {
                ull r = (i < other._digits.size()) ? other._digits[i] : 0;
                ull diff = result._digits[i] - r - borrow;

				if (result._digits[i] >= r + borrow) {
					diff = BASE - r - borrow;
					diff += result._digits[i];
					borrow = 0;
				} else {
					diff = result._digits[i] - r - borrow;
					borrow = 1;
				}
                result._digits[i] = static_cast<unsigned int>(diff);
            }
        } else {
			big_int temp (other - result);
			temp._sign = temp._sign ? false : true;
			return temp;
        }
    }

    result.normalize();
    return result;
}

big_int big_int::operator*(const big_int &other) const
{
	if (!(*this) || !other)
		return big_int(0);

	std::vector<unsigned int, pp_allocator<unsigned int>> result;
	result.resize(_digits.size() + other._digits.size(), 0);

	constexpr size_t half_bits = sizeof(unsigned int) * 4;

	constexpr unsigned int half_mask = __detail::generate_half_mask();

	constexpr uint64_t full_mask = (static_cast<uint64_t>(half_mask) << half_bits) | half_mask;

	constexpr size_t full_bits = half_bits * 2;

	for (size_t i = 0; i < _digits.size(); ++i)
	{
		uint64_t carry = 0;
		for (size_t j = 0; j < other._digits.size() || carry; ++j)
		{
			uint64_t multiplier = (j < other._digits.size()) ? other._digits[j] : 0;
			uint64_t cur = result[i + j] + static_cast<uint64_t>(_digits[i]) * multiplier + carry;
			result[i + j] = static_cast<unsigned int>(cur & full_mask);
			carry = cur >> full_bits;
		}
	}

	while (result.size() > 1 && result.back() == 0)
		result.pop_back();

	bool res_sign = (_sign == other._sign);
	return big_int(std::move(result), res_sign);
}

big_int &big_int::operator*=(const big_int &other) &
{
	*this = *this * other;
	return *this;
}

big_int big_int::operator/(const big_int &other) const
{
	if (other == big_int(0))
		throw std::runtime_error("Division by zero");

	big_int dividend = this->abs();
	big_int divisor = other.abs();

	if (dividend < divisor)
		return big_int(0);

	size_t n = dividend._digits.size();
	big_int quotient;
	quotient._digits.resize(n, 0);
	big_int R(0);

	for (int i = static_cast<int>(n) - 1; i >= 0; i--)
	{
		R = R << sizeof(unsigned int) * 8;
		R += big_int(dividend._digits[i]);
		unsigned long long left = 0, right = std::numeric_limits<unsigned int>::max();
		unsigned int q_digit = 0;
		while (left <= right)
		{
			unsigned long long mid = left + (right - left) / 2;
			big_int mid_val(mid);
			big_int candidate = divisor * mid_val;
			if (candidate <= R)
			{
				q_digit = static_cast<unsigned int>(mid);
				left = mid + 1;
			}
			else
			{
				right = mid - 1;
			}
		}
		quotient._digits[i] = q_digit;
		R = R - (divisor * big_int(q_digit));
	}

	quotient._sign = (this->_sign == other._sign);
	quotient.normalize();
	return quotient;
}

big_int big_int::operator%(const big_int &other) const
{
	if (other == big_int(0))
		throw std::runtime_error("Modulo by zero");
	big_int quotient = *this / other;
	big_int product = quotient * other;
	big_int remainder = *this - product;

	if (remainder._digits.size() == 1 && remainder._digits[0] == 0)
		remainder._sign = true;
	else
		remainder._sign = this->_sign;

	return remainder;
}

big_int big_int::operator&(const big_int &other) const
{
	big_int result;

	if (!_sign && other._sign)
		result._sign = true;
	else
		result._sign = _sign;

	size_t min_size = std::min(_digits.size(), other._digits.size());
	result._digits.resize(min_size);

	for (size_t i = 0; i < min_size; ++i)
	{
		result._digits[i] = _digits[i] & other._digits[i];
	}

	return result;
}

big_int big_int::operator|(const big_int &other) const { return ~((~*this) & (~other)); }

big_int big_int::operator^(const big_int &other) const { return ((~*this) & other) | (*this & (~other)); }

big_int big_int::operator<<(size_t shift) const
{
	big_int result = *this;

	if (shift / (8 * sizeof(unsigned int)) > 0)
	{
		size_t n = shift / (8 * sizeof(unsigned int));
		std::vector<unsigned int> vec(n, 0);
		result._digits.insert(result._digits.begin(), vec.begin(), vec.end());
		shift %= 8 * sizeof(unsigned int);
	}

	if (shift == 0)
		return result;

	unsigned int c = 0;

	for (auto &num : result._digits)
	{
		auto tmp = num;
		num = (num << shift) | c;
		c = tmp >> (8 * sizeof(unsigned int) - shift);
	}

	if (c != 0)
		result._digits.push_back(c);

	return result;
}

big_int big_int::operator>>(size_t shift) const
{
	big_int result(*this);
	if (shift / (8 * sizeof(unsigned int)) > 0)
	{
		size_t n = shift / (8 * sizeof(unsigned int));

		if (n >= result._digits.size())
		{
			result._sign = true;
			result._digits.clear();
			result._digits.push_back(0);
			return result;
		}

		result._digits.erase(result._digits.begin(), result._digits.begin() + n);
		shift %= 8 * sizeof(unsigned int);
	}

	if (shift == 0)
		return result;

	unsigned int c = 0;

	for (auto &num : std::views::reverse(result._digits))
	{
		auto tmp = num;
		num = (num >> shift) | c;
		c = tmp << (8 * sizeof(unsigned int) - shift);
	}

	return result;
}

big_int &big_int::operator%=(const big_int &other) &
{
	*this = *this % other;
	return *this;
}

big_int big_int::operator~() const
{
    big_int result = *this;

    for (auto &digit : result._digits) {
        digit = ~digit;
    }

    result._sign = !result._sign;
    return result;
}

big_int &big_int::operator&=(const big_int &other) &
{
	*this = *this & other;
	return *this;
}

big_int &big_int::operator|=(const big_int &other) &
{
	*this = *this | other;
	return *this;
}

big_int &big_int::operator^=(const big_int &other) &
{
	*this = *this ^ other;
	return *this;
}

big_int &big_int::operator<<=(size_t shift) &
{
	*this = *this << shift;
	return *this;
}

big_int &big_int::operator>>=(size_t shift) &
{
	*this = *this >> shift;
	return *this;
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) &
{
	throw not_implemented("big_int &big_int::plus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) &
{
	throw not_implemented("big_int &big_int::minus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::operator/=(const big_int &other) &
{
	*this = *this / other;
	return *this;
}

std::string big_int::to_string() const
{
	if (_digits.size() == 1 && _digits[0] == 0)
		return "0";

	std::vector<unsigned int, pp_allocator<unsigned int>> temp(_digits);

	ull base = static_cast<ull>(1) << (sizeof(unsigned int) * 8);

	std::string result;

	while (!(temp.size() == 1 && temp[0] == 0))
	{
		unsigned int carry = 0;
		for (int i = static_cast<int>(temp.size()) - 1; i >= 0; --i)
		{
			uint64_t current = static_cast<uint64_t>(carry) * base + temp[i];
			temp[i] = static_cast<unsigned int>(current / 10);
			carry = static_cast<unsigned int>(current % 10);
		}
		result.push_back('0' + carry);
		while (temp.size() > 1 && temp.back() == 0)
			temp.pop_back();
	}

	std::reverse(result.begin(), result.end());

	if (!_sign)
		result.insert(result.begin(), '-');

	return result;
}

std::ostream &operator<<(std::ostream &stream, const big_int &value)
{
	stream << value.to_string();
	return stream;
}

std::istream &operator>>(std::istream &stream, big_int &value)
{
	std::string s;
	stream >> s;
	value = big_int(s);
	return stream;
}

bool big_int::operator==(const big_int &other) const noexcept
{
	return (*this <=> other) == std::strong_ordering::equal;
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign)
	: _sign(sign), _digits(digits)
{
	normalize();
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign) noexcept
	: _sign(sign), _digits(std::move(digits))
{
	normalize();
}

big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int> alloc)
	: _sign(true), _digits(alloc)
{
	if (num.empty())
	{
		throw std::invalid_argument("Empty string");
	}
	if (radix < 2 || radix > 36)
	{
		throw std::invalid_argument("Radix must be between 2 and 36");
	}
	if (!validate_str(num, radix))
	{
		throw std::invalid_argument("Invalid characters for the given radix.");
	}

	size_t start_idx = (num[0] == '-') ? 1 : 0;
	_sign = (start_idx == 0);

	if (start_idx >= num.size())
	{
		throw std::invalid_argument("Number string is too short.");
	}
	bool original_sign = (start_idx == 0);
	big_int base(radix, alloc);
	big_int value(0, alloc);

	for (size_t i = start_idx; i < num.size(); ++i)
	{
		char ch = std::toupper(static_cast<unsigned char>(num[i]));
		unsigned int digit = (std::isdigit(ch) ? (ch - '0') : (ch - 'A' + 10));

		value *= base;
		value += big_int(digit, alloc);
	}

	_digits = value._digits;
	_sign = (_digits.size() == 1 && _digits[0] == 0) ? true : original_sign;

	normalize();
}

big_int::big_int(pp_allocator<unsigned int> alloc) : _sign(true), _digits({0}, alloc) {}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) &
{
	switch (rule)
	{
	case multiplication_rule::trivial:
		*this = *this * other;
		break;
	case multiplication_rule::Karatsuba:
		*this = karatsuba_multiply(other);
		break;
	case multiplication_rule::SchonhageStrassen:
		throw not_implemented(
			"big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) &",
			"Schonhage-Strassen multiplication not implemented yet.");
	default:
		throw std::invalid_argument("Unknown multiplication rule.");
	}
	return *this;
}

big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &
{
	if (other == big_int("0"))
	{
		throw std::logic_error("Division by zero");
	}

	switch (rule)
	{
	case division_rule::trivial:
		*this = *this / other;
		break;
	case division_rule::Newton:
		throw not_implemented(
			"big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &",
			"Newton division not implemented yet.");
	case division_rule::BurnikelZiegler:
		throw not_implemented(
			"big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &",
			"Burnikel-Ziegler division not implemented yet.");
	default:
		throw std::invalid_argument("Unknown division rule.");
	}
	return *this;
}

big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &
{
	throw not_implemented("big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &",
						  "your code should be here...");
}

big_int operator""_bi(unsigned long long n) {}

big_int big_int::karatsuba_multiply(const big_int &other) const
{
	if (_digits.size() <= 1 || other._digits.size() <= 1)
	{
        return *this * other;
	}

	size_t m = std::max(_digits.size(), other._digits.size());
	size_t m2 = m / 2;

	big_int r1, l1, r2, l2;

	if (_digits.size() > m2)
	{
		auto mid = _digits.begin() + m2;
		l1 = big_int(std::vector<unsigned int>(_digits.begin(), mid), true);
		r1 = big_int(std::vector<unsigned int>(mid, _digits.end()), true);
	}
	else
	{
		l1 = *this;
		r1 = big_int(0);
	}

	if (other._digits.size() > m2)
	{
		auto mid = other._digits.begin() + m2;
		l2 = big_int(std::vector<unsigned int>(other._digits.begin(), mid), true);
		r2 = big_int(std::vector<unsigned int>(mid, other._digits.end()), true);
	}
	else
	{
		l2 = other;
		r2 = big_int(0);
	}

	big_int z0 = l1.karatsuba_multiply(l2);
	big_int z1 = (l1 + r1).karatsuba_multiply(l2 + r2);
	big_int z2 = r1.karatsuba_multiply(r2);

	big_int middle = z1 - z2 - z0;

	big_int result = (z2 << (2 * m2 * sizeof(unsigned int) * 8)) +
					 (middle << (m2 * sizeof(unsigned int) * 8)) +
					 z0;

	result._sign = (_sign == other._sign);
	result.normalize();

	return result;
}
