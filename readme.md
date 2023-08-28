# readme
deduplicationのc++実装

参考
https://github.com/HojiChar/HojiChar/blob/main/hojichar/filters/deduplication.py

https://arxiv.org/abs/2107.06499


```
Usage: ./deduplicate <start> <end> <inputDir> <outputDir> <processedHashesDir>
```


指定したディレクトリ<inputDir>内のjsonlファイルに対してdedup処理を行い、指定したディレクトリ<outputDir>に出力する。  
ディレクトリのファイルは、1.jsonl、2.jsonl、を想定。  
startとendで処理したいファイルのindexを指定する。  

<processedHashesDir>でblacklist_dirを指定できる。  
これまで弾いた文章を記録として、1.txt、2.txt...が出力される。  
初めて動かす場合はディレクトリは空で動かす。  


## 準備

json dump用に`nlohmann/json`を使う。  
https://github.com/nlohmann/json  

```
sudo apt install nlohmann-json3-dev
```

`simdjson`で高速にjsonlファイルを処理できるらしい

https://github.com/simdjson/simdjson

```
wget https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h 
wget https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.cpp 
```


文章のハッシュ計算用に`MurmurHash`を使う。  
https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

```
git clone https://github.com/aappleby/smhasher.git
```

```
make
```

## ハッシュ値の計算
ハッシュ値の計算のみ並列化。デフォルトで2thread。このあたりは環境によって変える。

`NUM_WORKERS = 2;`


hash計算のパラメタ。参考実装より少し小さめ

```
N = 5
N_MINHASH = 100
N_BUCKET = 10
BUCKET_SIZE = 10
```

Ngramに分割、N_MINHASHのハッシュ値から、N_BUKET個のハッシュ値を生成 

https://github.com/HojiChar/HojiChar/blob/main/hojichar/filters/deduplication.py