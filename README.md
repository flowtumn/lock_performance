Qiitaに投稿しました、lockのパフォーマンスを測定するプログラムです。

http://qiita.com/flowtumn/items/9cc4865a408eb695138d

## 使い方

```
$ git clone [lock_performance repository]
$ cd ./lock_performance
$ mkdir ./out && cd ./out
$ cmake -DCMAKE_BUILD_TYPE=Release ../
$ cmake --build .
$ ctest -V
```

[![Build Status](https://travis-ci.org/flowtumn/lock_performance.svg?branch=master) [![Build Status](https://travis-ci.org/flowtumn/lock_performance.svg?branch=develop)
