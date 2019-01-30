int ret() {
    int ans;
    ans = 100;
    return ans;
}

int main() {
    int a, b;
    a = 10;
    b = ret() + ret();
    return b;
}
