int fibo(int n) {
    if(n < 2)
        return 1;
    else
        return fibo(n - 2) + fibo(n - 1);
}

int main() {
    return fibo(5);
}
