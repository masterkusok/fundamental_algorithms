#include "../include/fraction.h"

big_int gcd(big_int a, big_int b) {
    while (b != 0_bi) {
        big_int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}


void fraction::optimise() {
    if (_denominator == 0_bi)
        throw std::runtime_error("divsion by zero");

    if (_denominator < 0_bi) {
        _numerator = big_int(-1) *_numerator;
        _denominator = big_int(-1) * _denominator;
    }

    big_int common = gcd(_numerator.abs(), _denominator.abs());
    if (common != 0_bi && common != 1_bi) {
        _numerator /= common;
        _denominator /= common;
    }
}

template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f&& numerator, s&& denominator)
    : _numerator(std::forward<f>(numerator)),
      _denominator(std::forward<s>(denominator))
{
    if (_denominator == 0_bi)
        throw std::invalid_argument("Denominator cannot be zero");
    optimise();
}

fraction::fraction(pp_allocator<big_int::value_type>)
    : _numerator(0_bi), _denominator(1_bi) {}

fraction& fraction::operator+=(const fraction& other) & {
    big_int lcm = (_denominator * other._denominator) / gcd(_denominator, other._denominator);
    _numerator = _numerator * (lcm / _denominator) + other._numerator * (lcm / other._denominator);
    _denominator = lcm;
    optimise();
    return *this;
}

fraction fraction::operator+(const fraction& other) const {
    fraction result(*this);
    result += other;
    return result;
}

fraction &fraction::operator-=(fraction const &other) &
{
    big_int lcm = (_denominator * other._denominator) / gcd(_denominator, other._denominator);
    _numerator = _numerator * (lcm / _denominator) - other._numerator * (lcm / other._denominator);
    _denominator = lcm;
    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const
{
    fraction result(*this);
    result -= other;
    return result;
}

fraction &fraction::operator*=(fraction const &other) &
{
    _numerator *= other._numerator;
    _denominator *= other._denominator;
    optimise();
    return *this;
}

fraction fraction::operator*(fraction const &other) const
{
    fraction result(*this);
    result *= other;
    return result;
}

fraction &fraction::operator/=(fraction const &other) &
{
    _numerator *= other._denominator;
    _denominator *= other._numerator;
    optimise();
    return *this;
}

fraction fraction::operator/(fraction const &other) const
{
    fraction result(*this);
    result /= other;
    return result;
}

bool fraction::operator==(fraction const &other) const noexcept
{
    return _numerator == other._numerator && _denominator == other._denominator;
}

std::partial_ordering fraction::operator<=>(const fraction& other) const noexcept {
    big_int left = _numerator * other._denominator;
    big_int right = other._numerator * _denominator;
    return left <=> right;
}

std::ostream& operator<<(std::ostream& stream, const fraction& obj) {
    stream << obj._numerator << "/" << obj._denominator.abs();
    return stream;
}

std::istream& operator>>(std::istream& stream, fraction& obj) {
    char slash;
    stream >> obj._numerator >> slash >> obj._denominator;
    if (slash != '/' || obj._denominator == 0_bi)
        stream.setstate(std::ios::failbit);
    obj.optimise();
    return stream;
}

std::string fraction::to_string() const
{
    return "(" + _numerator.to_string()  + ")/(" + _denominator.to_string() + ")";
}

fraction fraction::abs() const {
    return fraction(_numerator.abs(), _denominator);
}

bool fraction::is_zero() const {
    return _numerator == 0_bi;
}

fraction fraction::sin(const fraction& epsilon) const {
    if (is_zero()) return fraction(0_bi, 1_bi);

    fraction term = *this;
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int factorial = 1_bi;
    big_int n = 1_bi;

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) / fraction(-(i - 1) * i, 1_bi);
        factorial *= i * (i - 1);
        term = term / fraction(factorial, 1_bi);

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::cos(const fraction& epsilon) const {
    if (is_zero()) return fraction(1_bi, 1_bi);

    fraction term(1_bi, 1_bi);
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int factorial = 1_bi;
    big_int n = 0_bi;

    for (int i = 2; ; i += 2) {
        term = (term * x_sq) / fraction(-(i - 1) * i, 1_bi);
        factorial *= i * (i - 1);
        term = term / fraction(factorial, 1_bi);

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::tg(const fraction& epsilon) const {
    fraction cos_val = cos(epsilon);
    if (cos_val.is_zero())
        throw std::runtime_error("Tangent is undefined for this angle");
    return sin(epsilon) / cos_val;
}

fraction fraction::ctg(const fraction& epsilon) const {
    fraction sin_val = sin(epsilon);
    if (sin_val.is_zero())
        throw std::runtime_error("Cotangent is undefined for this angle");
    return cos(epsilon) / sin_val;
}

fraction fraction::sec(const fraction& epsilon) const {
    fraction cos_val = cos(epsilon);
    if (cos_val.is_zero())
        throw std::runtime_error("Secant is undefined for this angle");
    return fraction(1_bi, 1_bi) / cos_val;
}

fraction fraction::cosec(const fraction& epsilon) const {
    fraction sin_val = sin(epsilon);
    if (sin_val.is_zero())
        throw std::runtime_error("Cosecant is undefined for this angle");
    return fraction(1_bi, 1_bi) / sin_val;
}

fraction fraction::arcsin(const fraction& epsilon) const {
    if (is_zero()) return fraction(0_bi, 1_bi);

    fraction term = *this;
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int n = 1_bi;

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) * fraction(n * n, i * i);
        n += 1_bi;

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::arccos(const fraction& epsilon) const {
    fraction pi_half(314159265_bi, 200000000_bi);
    return pi_half - arcsin(epsilon);
}

fraction fraction::arctg(const fraction& epsilon) const {
    if (is_zero()) return fraction(0_bi, 1_bi);

    fraction term = *this;
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int n = 1_bi;

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) / fraction(-i, 1_bi);

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::root(size_t degree, const fraction& epsilon) const {
    if (*this < fraction(0_bi, 1_bi) && degree % 2 == 0)
        throw std::runtime_error("Even root of a negative number is undefined");

    fraction guess = (*this + fraction(1_bi, 1_bi)) / fraction(2_bi, 1_bi);
    fraction prev_guess;

    do {
        prev_guess = guess;
        guess = (guess * fraction(degree - 1, 1_bi) + (*this / guess.pow(degree - 1))) / fraction(degree, 1_bi);
    } while ((guess - prev_guess).abs() > epsilon);

    return guess;
}

fraction fraction::pow(size_t degree) const {
    fraction result(1_bi, 1_bi);
    for (size_t i = 0; i < degree; ++i) {
        result *= *this;
    }
    return result;
}

fraction fraction::ln(const fraction& epsilon) const {
    if (*this <= fraction(0_bi, 1_bi))
        throw std::runtime_error("Logarithm is undefined for non-positive values");

    fraction x = (*this - fraction(1_bi, 1_bi)) / (*this + fraction(1_bi, 1_bi));
    fraction term = x;
    fraction result = term;
    fraction x_sq = x * x;
    big_int n = 1_bi;

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) * fraction(n, i);
        n += 1_bi;

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result * fraction(2_bi, 1_bi);
}

fraction fraction::lg(const fraction& epsilon) const {
    fraction ln_val = ln(epsilon);
    fraction ln_10(230258509_bi, 100000000_bi);
    return ln_val / ln_10;
}

fraction fraction::log2(const fraction& epsilon) const {
    fraction ln_val = ln(epsilon);
    fraction ln_2(69314718_bi, 100000000_bi);
    return ln_val / ln_2;
}
