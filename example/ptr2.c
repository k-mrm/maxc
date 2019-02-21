int main() {
    int *a, b = 50;
    int *p;
    a = &b;
    p = a + 1;
    return *(p + 1);
}
