int add(int a1, int a2) {
    return a1 + a2;
}

int main() {
    int a, ans;
    a = 10;
    ans = add(30, add(10, 20));
    return ans;
}
