#include "../include/fraction.h"

big_int gcd(big_int a, big_int b) {
    while (b != big_int("0")) {
        big_int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void fraction::optimise() {
    if (_denominator == big_int("0"))
        throw std::runtime_error("divsion by zero");

    if (_denominator < big_int("0")) {
        _numerator = big_int("-1") * _numerator;
        _denominator = big_int("-1") * _denominator;
    }

    big_int common = gcd(_numerator.abs(), _denominator.abs());
    if (common != big_int("0") && common != big_int("1")) {
        _numerator /= common;
        _denominator /= common;
    }
}

template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f&& numerator, s&& denominator)
    : _numerator(std::forward<f>(numerator)),
      _denominator(std::forward<s>(denominator))
{
    if (_denominator == big_int("0"))
        throw std::invalid_argument("Denominator cannot be zero");
    optimise();
}

fraction::fraction(pp_allocator<big_int::value_type>)
    : _numerator(big_int("0")), _denominator(big_int("1")) {}

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
    stream << '(' << obj._numerator << '/' << obj._denominator.abs() << ')';
    return stream;
}

std::istream& operator>>(std::istream& stream, fraction& obj) {
    char slash;
    stream >> obj._numerator >> slash >> obj._denominator;
    if (slash != '/' || obj._denominator == big_int("0"))
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
    return _numerator == big_int("0");
}

fraction fraction::sin(const fraction& epsilon) const {
    if (is_zero()) return fraction(big_int("0"), big_int("1"));

    fraction term = *this;
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int factorial = big_int("1");
    big_int n = big_int("1");

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) / fraction(big_int("-1") * big_int(std::to_string((i - 1) * i)), big_int("1"));
        factorial *= big_int(std::to_string(i * (i - 1)));
        term = term / fraction(factorial, big_int("1"));

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::cos(const fraction& epsilon) const {
    if (is_zero()) return fraction(big_int("1"), big_int("1"));

    fraction term(big_int("1"), big_int("1"));
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int factorial = big_int("1");
    big_int n = big_int("0");

    for (int i = 2; ; i += 2) {
        term = (term * x_sq) / fraction(big_int("-1") * big_int(std::to_string((i - 1) * i)), big_int("1"));
        factorial *= big_int(std::to_string(i * (i - 1)));
        term = term / fraction(factorial, big_int("1"));

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
    return fraction(big_int("1"), big_int("1")) / cos_val;
}

fraction fraction::cosec(const fraction& epsilon) const {
    fraction sin_val = sin(epsilon);
    if (sin_val.is_zero())
        throw std::runtime_error("Cosecant is undefined for this angle");
    return fraction(big_int("1"), big_int("1")) / sin_val;
}

fraction fraction::arcsin(const fraction& epsilon) const {
    if (is_zero()) return fraction(big_int("0"), big_int("1"));

    fraction term = *this;
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int n = big_int("1");

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) * fraction(n * n, big_int(std::to_string(i * i)));
        n += big_int("1");

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::arccos(const fraction& epsilon) const {
    fraction pi_half(big_int("314159265"), big_int("200000000"));
    return pi_half - arcsin(epsilon);
}

fraction fraction::arctg(const fraction& epsilon) const {
    if (is_zero()) return fraction(big_int("0"), big_int("1"));

    fraction term = *this;
    fraction result = term;
    fraction x_sq = (*this) * (*this);
    big_int n = big_int("1");

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) / fraction(big_int("-1") * big_int(std::to_string(i)), big_int("1"));

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result;
}

fraction fraction::root(size_t degree, fraction const &epsilon) const
{
    if (degree == 0)
    {
        throw std::invalid_argument("Zero root is undefined");
    }

    if (_numerator < big_int(0) && degree % 2 == 0)
    {
        throw std::domain_error("Even root of negative number");
    }

    fraction guess = (*this).abs();
    fraction prev_guess;
    fraction power;

    do
    {
        prev_guess = guess;
        power = guess.pow(degree - 1);
        guess = fraction(degree - 1, 1) * guess + (*this) / power;
        guess /= fraction(degree, 1);
    } while ((guess - prev_guess).abs() > epsilon);

    return guess;
}

fraction fraction::pow(size_t degree) const {
    fraction result(big_int("1"), big_int("1"));
    for (size_t i = 0; i < degree; ++i) {
        result *= *this;
    }
    return result;
}

fraction fraction::ln(const fraction& epsilon) const {
    if (*this <= fraction(big_int("0"), big_int("1")))
        throw std::runtime_error("Logarithm is undefined for non-positive values");

    fraction x = (*this - fraction(big_int("1"), big_int("1"))) / (*this + fraction(big_int("1"), big_int("1")));
    fraction term = x;
    fraction result = term;
    fraction x_sq = x * x;
    big_int n = big_int("1");

    for (int i = 3; ; i += 2) {
        term = (term * x_sq) * fraction(n, big_int(std::to_string(i)));
        n += big_int("1");

        if (term.abs() < epsilon) break;
        result += term;
    }
    return result * fraction(big_int("2"), big_int("1"));
}

fraction fraction::lg(const fraction& epsilon) const {
    fraction ln_val = ln(epsilon);
    fraction ln_10(big_int("230258509"), big_int("100000000"));
    return ln_val / ln_10;
}

fraction fraction::log2(const fraction& epsilon) const {
    fraction ln_val = ln(epsilon);
    fraction ln_2(big_int("69314718"), big_int("100000000"));
    return ln_val / ln_2;
}
