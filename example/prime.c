int isprime(int n) {
    int i, ans = 0, flag = 0, f = 0;

    if(n % 2 == 0)
        return 0;
    else {
        for(i = 3; i < n; ++i) {
            ans = n % i;
            if(ans == 0 && f == 0) {
                flag = 1;
                f = 1;
            }
            else if(f == 0)
                flag = 0;
        }
    }

    if(flag)
        return 0;   //no prime
    else
        return n;   //is prime
}

int main() {
    return isprime(137);
}
