# readme
deduplicationのc++実装

参考
https://github.com/HojiChar/HojiChar/blob/main/hojichar/filters/deduplication.py

https://arxiv.org/abs/2107.06499

指定したディレクトリ内のjsonlファイルに対してdedup処理を行い、指定したディレクトリに出力する

## 準備

```
sudo apt install nlohmann-json3-dev
```

clone this repository
https://github.com/aappleby/smhasher.git


```
make
```

## run

```
./deduplicate {target_dir} {output_dir}
```