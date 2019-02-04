int ret() {
    int a;
    a = 30;
    if(a == 30)
        return 1;
    else
        return 0;
}

int main() {
    int a = 0;

    if(ret() && a) {
        return a;
    }
    else if(ret() || a) {
        a = 105;
        return a;
    }
    else {
        return 200;
    }
}
