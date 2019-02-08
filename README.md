# maxc
A cheep programming language written in C++ from scratch(unfinished)


## 説明です
C++で出来たmaxc言語(自作)のコンパイラです(最初はインタプリタの予定だった)。<br>
手探りでこれをやっていますが、果たしてどこまでいけるのでしょうか。

Twitter: [@cmpl_error](https://twitter.com/cmpl_error)

### 今できること(随時更新)

#### 2019-01-06 (36 commits)
- `+-*/()`を用いた四則演算が出来る(結果をraxレジスタに返してあげることが出来る)<br>
- `//`コメントに対応

test
```
(4 + 10) * (2 + 9)
//answer is 154!
```

output
```
$ ./maxc example/test > a.s
$ gcc a.s
$ ./a.out ; echo $?
154
```

#### 2019-01-09 (40 commits)
- `%`演算子を追加<br>

#### 2019-01-17 (56 commits)
- 変数に対応(なお一個しか使えない)

example/m.maxc
```
hoge = 10;
10 * hoge;
```

output
```
$ ./maxc example/m.maxc > a.s
$ gcc a.s
$ ./a.out ; echo $?
100
```

#### 2019-01-19 (60 commits)
- 複数個の変数に対応

example/m.maxc
```
var a, b, c, d;
a = 10;
b = 5;
c = 15;
d = 30;
a * b + c - d;
```

output
```
$ ./maxc example/m.maxc > a.s
$ gcc a.s
$ ./a.out ; echo $?
35
```

#### 2019-01-22 (67 commits - [c34a2f6](https://github.com/k-mrm/maxc/commit/c34a2f6ffe36d13ca0ebdf0351d97b6ac93d524f))
- 関数定義を実装・main関数に対応

example/func.maxc
```
int main() {
    4 * 2 + 10;
}
```

output
```
$ ./maxc example/func.maxc > a.s
$ gcc a.s
$ ./a.out ; echo $?
18
```
~~薄々感づいてたけどなんかこれCコンパイラになってきたな…~~

#### 2019-1-29 (77 commits - [7b49118](https://github.com/k-mrm/maxc/tree/7b491185b72ad5a4fd253f78e246341d55c03e62))
- 引数を取らない関数呼び出しに対応

example/funccall.c
```
int ret() {
    int ans;
    ans = 100
    return ans;
}

int main() {
    int a;

    a =  ret();
    return a;
}
```

output
```
$ ./maxc exaple/funccall.c > a.s
$ gcc a.s
$ ./a.out ; echo $?
100
```

#### 2019-1-30 (81 commits - [5444d82](https://github.com/k-mrm/maxc/commit/5444d82158db362d0812c224310a1daa4646e453))
- if文に対応(1行のみ)

example/if.c
```
int main() {
    int a;

    if(0)
        a = 10;
    else if(1)
        a = 50;
    else
        a = 20;

    return a;
}
```

output
```
$ ./maxc.sh example/if.c
$ ./a.out ; echo $?
50
```

#### 2019-1-30 (86 commits - [9cb5d94](https://github.com/k-mrm/maxc/commit/9cb5d94cac51a2d89fc4b16d2d2f3bacb3ee9a59))
- if文に対応(複数の文使用可)

example/if.c
```
int ret() {
    int a;
    a = 10;
    if(a == 10)
        return 1;
    else
        return 0;
}

int main() {
    int a;
    a = 10;
    if(ret()) {
        return a;
    }
    else if(a == 20) {
        a = a * 5;
        return a;
    }
    else {
        return 200;
    }
}
```

output
```
$ sh maxc.sh example/if.c
$ ./a.out ; echo $?
10
```

#### 2019-2-01 (89 commits - [e2fa37b](https://github.com/k-mrm/maxc/commit/e2fa37bf61493ea30c5c2068f99d205d8a1b22e5))
- while文に対応

example/while.c
```
int main() {
    int a, sum;
    a = 0; sum = 0;
    while(a != 10) {
        sum = sum + a;
        a = a + 1;
    }

    return sum;
}
```

output
```
$ sh maxc.sh example/while.c
$ ./a.out ; echo $?
45
```

#### 2019-02-03 (95 commits - [91f9e89](https://github.com/k-mrm/maxc/commit/91f9e89d3d8a49d7bb52fb44a928bd59f1100bb5))
- 引数あり(6個まで)の関数呼び出しに対応
- 変数宣言時の初期化に対応

example/funccall2.c
```
int add(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

int main() {
    return add(20, 10, 15, 1, 48, 30);
}
```

output
```
$ sh maxc.sh example/funccall2.c
$ ./a.out ; echo $?
124
```

#### 2019-02-04 (97 commits - [bca36f6](https://github.com/k-mrm/maxc/tree/bca36f6044aeb7106b293139f402e95057bb75c7))
- for文に対応

example/for.c
```
int main() {
    int i, sum = 0;
    for(i = 0; i <= 15; i = i + 1) {
        sum = sum + i;
    }

    return sum;
}
```

output
```
$ sh maxc.sh example/for.c
$ ./a.out; echo $?
120
```

#### 2019-02-09 (108 commits - [237b142](https://github.com/k-mrm/maxc/commit/237b1420c01f497d1cded1d4b089b4bc85b498a8))
- if式に対応
- and, orに対応(&&、||と同じ)
- 前置インクリメント、デクリメントに対応

example/ifexpr.c
```
int main() {
    int i, c = 10, x = 0;
    i = if(c and x) {
        return 10;
    } else if(c or x){
        return 114;
    } else {
        return 0;
    }

    ++i;

    return i;
}
```

output
```
115
```

某に「これ自作言語じゃなくてCコンパイラやん」と言われてしまったためC言語には無いものをつけてみました。

## 参考ドキュメント・リポジトリ集
- [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook/)
- [rui314/9cc](https://github.com/rui314/9cc)
- [rui314/8cc](https://github.com/rui314/8cc)
- [maekawatoshiki/qcc](https://github.com/maekawatoshiki/qcc)
