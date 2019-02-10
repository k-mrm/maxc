int ret2(int n) {
    return n * 3;
}

int ret(int n) {
    return ret2(n - 1) + ret2(n - 2);
}

int main() {
    int a = 50;
    return a;
}
