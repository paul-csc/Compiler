{
    int base;
    int exp;
    int result;

    base = 5;
    exp = 4;

    result = 1;
    while (exp) {
        result = result * base;
        exp = exp - 1;
    }

    result = result;
}