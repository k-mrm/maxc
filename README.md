# maxc
[![](http://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)

maxc is a programming language in C.

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
    if n <= 1 {
        return n;
    }

    return fibo(n - 2) + fibo(n - 1);
}

println(fibo(30));
30.fibo().println();
30.fibo.println;
```
```
832040
832040
832040
```

## Document(Japanese)
https://admarimoin.hatenablog.com/entry/2019/08/28/155346

