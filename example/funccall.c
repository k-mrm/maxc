int max(int a1, int a2) {
    if(a1 < a2)
        return a2;
    else
        return a1;
}

int main() {
    int a, ans;
    a = 200 - 10;
    ans = max(a, 40);
    return ans;
}
