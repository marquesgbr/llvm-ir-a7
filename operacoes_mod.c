#include <stdio.h>

int soma(int a, int b) {
    return a + b;
}

int multiplica(int x, int y) {
    return x * y;
}

int divide(int x, int y) {
    if (y == 0) return 0;
    return x / y;
}

int calcula(int valor) {
    int temp = soma(valor, 3);
    if (temp % 2 == 0) {
        return multiplica(temp, 4);
    } else {
        return divide(temp, 2);
    }
}

int main() {
    int resultado = calcula(5);
    printf("Resultado: %d\n", resultado);
    return 0;
}
