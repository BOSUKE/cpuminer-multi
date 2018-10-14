FPGA(bitcoin_miner)対応 CPUMiner-Multi
==============

## これは何か？

電脳律速の同人誌「FPGA×仮想通貨」でPYNQ-Z1のFPGA(PL)に
実装したBitcoinのマイニングハードウエア(bitcoin_miner)を利用するように改造した
cpuminer。

同人誌「FPGA×仮想通貨」について : https://d-rissoku.net/サークルサポート/fpga_x_仮想通貨/
bitcoin_miner のコード等 : https://github.com/BOSUKE/fpga_x_cryptocurrency
fork元 : https://github.com/tpruvot/cpuminer-multi

元々のcpuminer-multiは様々なアルゴリズムに対応しているが、この改造版cpuminer-multiは
sha256dをbitcoin_miner(FPGA)を用いてマイニングすることしか考えずに改造。

PYNQ-Z1で動作するUbuntu 16.04 LTS (GNU/Linux 4.9.0-xilinx armv7l)でのみ動作確認済み。

## 依存ライブラリなどの入手

fork元を参考にPYNQ-Z1上で次を実施。ただし、実際のところデフォルトのPYNQ-Z1のイメージを使っている場合、足りないのは libjansson-dev のみと思われる。

```
 sudo apt-get install automake autoconf pkg-config libcurl4-openssl-dev libjansson-dev libssl-dev libgmp-dev make g++
```

## ダウンロードとビルド

PYNQ-Z1上にて
```
git clone https://github.com/BOSUKE/cpuminer-multi
cd cpuminer-multi
./build.sh
```

## Walletとマイニングプールの準備

すでに準備がある人はそれを使えばOK。

準備がない人で、とりあえずbitcoin_minerを動かしてみたい人は https://www.bitaddress.org/ に行ってマウスをぐりぐり動かして、Bitcoin Addressを生成。SHAREとSECRETの値をメモっておくだけでOK。

## 実行

予め https://github.com/BOSUKE/fpga_x_cryptocurrency の  jupyter_notebooks/miner.py を用いてPYNQ-Z1を bitcoin_minerにコンフィグレーションした状態でPYNQ-Z1上で以下を実行。

```
sudo ./cpuminer -a sha256d -o stratum+tcp://（マイニングプール） -u (ユーザ名) -p (パスワード)
```

上でBitcoin Addressだけは作ったという人は、以下を実行すればとりあえずマイニング処理は動作します。

```
sudo ./cpuminer -a sha256d -o stratum+tcp://solo.ckpool.org:3333 -u （作成したBitcoin Address(SHARE))
```
