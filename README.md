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
- '%'演算子を追加<br>
