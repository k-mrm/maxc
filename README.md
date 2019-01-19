# maxc
A cheep programming language written in C++(unfinished)


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

## 参考ドキュメント・リポジトリ集
- [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook/)
- [rui314/9cc](https://github.com/rui314/9cc)
- [rui314/8cc](https://github.com/rui314/8cc)
- [maekawatoshiki/qcc](https://github.com/maekawatoshiki/qcc)
