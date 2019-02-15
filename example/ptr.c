int main(){
    int a = 0;
    int* pa;
    int** pp;

    pa = &a;
    pp = &pa;
    **pp = 49;
    return **pp;
}
