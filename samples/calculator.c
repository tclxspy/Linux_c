#include <stdio.h>

double calculator(double num_1, double num_2, char operate)
{
    switch (operate) {
    case '+':
	return num_1 + num_2;
    case '-':
	return num_1 - num_2;
    case '*':
	return num_1 * num_2;
    case '/':
	return num_1 / num_2;
    default:
	return -1;
    }
}

int main(int agrc, char *agv[])
{
    double a, b, c;
    char m;

    printf("input num_1:\n");
    scanf("%lf", &a);

    printf("input operate(+ - * /):\n");
    scanf("%c", &m);
    m = getchar();

    printf("input num_2:\n");
    scanf("%lf", &b);

    c = calculator(a, b, m);
    printf("%lf %c %lf = %lf\n", a, m, b, c);

    return 0;
}
