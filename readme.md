# readme
deduplicationのc++実装

参考
https://github.com/HojiChar/HojiChar/blob/main/hojichar/filters/deduplication.py

https://arxiv.org/abs/2107.06499

指定したディレクトリ内のjsonlファイルに対してdedup処理を行い、指定したディレクトリに出力する。  
ディレクトリのファイルは、1.jsonl、2.jsonl、...想定。  
blacklist_pathを指定すると、これまで弾いた文章を記録できる。  
初めて動かす場合は空のファイルを指定する。resumeしたいときは保存したblacklist_pathを指定。  

実行は開始indexと終了indexを指定する。

```
./deduplicate {start_index} {end_index} {target_dir} {output_dir} {blacklist_path}
```

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