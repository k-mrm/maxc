# maxc

A programming language(compiler and VM)

## Sample Code

hello.mxc
```
println("Hello, World!");
```
```
Hello, World!
```


fibo.mxc
```
fn fibo(n: int): int {
    if(n <= 1) {
        return n;
    }

    return fibo(n - 2) + fibo(n - 1);
}

println(fibo(30));
```
```
832040
```
