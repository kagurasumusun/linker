# linker

理論上最速を目指す高並列・ゼロコピーリンカ。

## 特徴
- **C++23**: 最新の言語機能を活用。
- **Zero-Copy**: mmap を使用した入力・出力 I/O。
- **Parallelism**: oneTBB を使用したシンボル解決・レイアウト・再配置の並列化。
- **Memory Optimization**: mimalloc による高速なメモリ割り当て。

## 構成
- `src/`: ソースコード
- `tests/`: テスト及びベンチマークスクリプト

## ビルド方法
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 使用方法
```bash
./fast-linker <input_files...>
```
