int main(){
    int a = 0;
    int* pa;

    pa = &a;
    *pa = 1;
    return *pa;
}
