#include <stdio.h>

int soma(int a, int b) {
    return a + b;
}

int multiplica(int x, int y) {
    return x * y;
}

int calcula(int valor) {
    if (valor > 10) {
        return multiplica(valor, 2);
    } else {
        return soma(valor, 5);
    }
}

int main() {
    int resultado = calcula(7);
    printf("Resultado: %d\n", resultado);
    return 0;
}
