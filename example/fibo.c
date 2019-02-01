int fibo(int n) {
    int ans;
    if(n < 2)
        return 1;
    else {
        ans = fibo(n - 2) + fibo(n - 1);
        return ans;
    }
}

int main() {
    int num;
    num = fibo(10);
    return num;
}
