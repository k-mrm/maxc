#include <stdio.h>

int main(void) {
    int a = 0;
    while(a < 100000000) {
        a++;
    }

    printf("%d", a);
}
