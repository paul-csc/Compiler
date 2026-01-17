
{
    int count;
    int a;
    int b;
    int temp;

    count = 30;
    a = 0;
    b = 1;
    count = count - 1;
    while (count) {
        temp = a + b;
        a = b;
        b = temp;

        count = count - 1;
    }

    b = b;
}