#include <gtest/gtest.h>
#include <fraction.h>
#include <iostream>

void print_fraction(const std::string& name, const fraction& f) {
    std::cout << name << " = " << f  << std::endl;
}

TEST(FractionTest, BasicArithmetic) {
    fraction a(big_int("1"), big_int("2"));
    fraction b(big_int("3"), big_int("4"));

    std::cout << "\n[Арифметика]\n";
    print_fraction("a", a);
    print_fraction("b", b);
    print_fraction("a + b", a + b);
    print_fraction("b - a", b - a);
    print_fraction("a * b", a * b);
    print_fraction("a / b", a / b);
}

TEST(FractionTest, Trigonometric) {
    fraction zero(big_int("0"), big_int("1"));
    fraction pi_over_4(big_int("785398163"), big_int("1000000000"));
    fraction epsilon(big_int("1"), big_int("1000000"));

    std::cout << "\n[Тригонометрия]\n";
    print_fraction("sin(0)", zero.sin(epsilon));
    print_fraction("cos(0)", zero.cos(epsilon));
    print_fraction("tg(π/4)", pi_over_4.tg(epsilon));
}

TEST(FractionTest, Logarithms) {
    fraction e(big_int("2718281828"), big_int("1000000000")); // e ≈ 2.71828
    fraction ten(big_int("10"), big_int("1"));
    fraction epsilon(big_int("1"), big_int("1000000"));

    std::cout << "\n[Логарифмы]\n";
    print_fraction("ln(e)", e.ln(epsilon));
    print_fraction("log10(100)", fraction(big_int("100"), big_int("1")).lg(epsilon));
}

TEST(FractionTest, Exceptions) {
    fraction zero(big_int("0"), big_int("1"));
    fraction negative(big_int("-4"), big_int("1"));
    fraction epsilon(big_int("1"), big_int("1000000"));

    std::cout << "\n[Исключения]\n";
    try {
        auto res = zero / zero;
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << std::endl;
    }

    try {
        auto res = negative.root(2, epsilon);
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << std::endl;
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    std::cout << "=== Результаты тестов ===" << std::endl;
    auto result = RUN_ALL_TESTS();
    std::cout << "\n=== Конец вывода ===" << std::endl;
    return result;
}
