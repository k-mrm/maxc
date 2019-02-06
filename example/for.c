int main() {
    int a = 1, b = 1, c = 1, x;
    for(b = 2; b < 10; ++b) {
        for(c = b + 1; c < 10; ++c) {
            x = a + a * b + a * b * c;
        }
    }
    return x;
}
