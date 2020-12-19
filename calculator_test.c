#include <stdint.h>
#include <assert.h>

int calculator_eval(const char *input, int64_t *result);

void
calculator_assert(const char *input, int64_t expected)
{
    int64_t result;
    assert(calculator_eval(input, &result) == 0);
    assert(result == expected);
}

int
main(int argc, char *argv[])
{
    // Number formatting.
    //
    calculator_assert("101", 257);
    calculator_assert("aaa", 2730);
    calculator_assert("0x101", 257);
    calculator_assert("0xaaa", 2730);
    calculator_assert("0n101", 101);
    calculator_assert("0b101", 5);
    calculator_assert("0101", 65);

    // Basic arithmetic.
    //
    calculator_assert("1 + 1", 2);
    calculator_assert("100 + 100", 512);
    calculator_assert("1 + 2 + 3", 6);

    calculator_assert("1 - 1", 0);
    calculator_assert("0 - 1", -1);
    calculator_assert("1 - 0", 1);
    calculator_assert("0 - 0", 0);
    calculator_assert("10 - 4 - 7", 5);

    calculator_assert("1 * 1", 1);
    calculator_assert("0 * 1", 0);
    calculator_assert("1 * 0", 0);
    calculator_assert("0 * 0", 0);
    calculator_assert("42 * 42", 4356);
    calculator_assert("3 * 100 * 100", 196608);

    calculator_assert("1 / 1", 1);
    calculator_assert("0 / 1", 0);
    calculator_assert("1 / 0", 0);
    calculator_assert("0 / 0", 0);
    calculator_assert("42 / 100", 0);
    calculator_assert("84 / 42", 2);

    calculator_assert("1 % 1", 0);
    calculator_assert("0 % 1", 0);
    calculator_assert("1 % 0", 0);
    calculator_assert("0 % 0", 0);
    calculator_assert("2 % 100", 2);
    calculator_assert("20 % 2", 0);

    // Bit operators.
    //
    calculator_assert("20 & 1", 0);
    calculator_assert("21 & 1", 1);

    calculator_assert("20 | 1", 33);
    calculator_assert("21 | 1", 33);

    calculator_assert("20 ^ 1", 33);
    calculator_assert("21 ^ 1", 32);

    calculator_assert("20 << 1", 64);
    calculator_assert("20 >> 1", 16);

    calculator_assert("~0", -1);
    calculator_assert("~20", -33);

    // Logic operators.
    //
    calculator_assert("1 || 0", 1);
    calculator_assert("1 && 0", 0);
    calculator_assert("!0", 1);
    calculator_assert("!1", 0);

    // Comparison operators.
    //
    calculator_assert("20 > 1", 1);
    calculator_assert("20 > 21", 0);
    calculator_assert("20 > 20", 0);

    calculator_assert("20 >= 1", 1);
    calculator_assert("20 >= 21", 0);
    calculator_assert("20 >= 20", 1);

    calculator_assert("20 < 1", 0);
    calculator_assert("20 < 21", 1);
    calculator_assert("20 < 20", 0);

    calculator_assert("20 <= 1", 0);
    calculator_assert("20 <= 21", 1);
    calculator_assert("20 <= 20", 1);

    calculator_assert("20 == 1", 0);
    calculator_assert("20 == 20", 1);

    calculator_assert("20 != 1", 1);
    calculator_assert("20 != 20", 0);

    // Expressions.
    //
    calculator_assert("1 + 2 * 3 / 4", 2);
    calculator_assert("3 + 4 % 10", 7);
    calculator_assert("~0b00 + 0n10 * 0x20 << 030 != 40 < 50", 1);
    calculator_assert("~0b00 + 0n10 * 0x20 << ((030 != 40) < 50)", 638);
    calculator_assert("(1) + (2) + (3)", 6);

    return 0;
}
