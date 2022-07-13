# BleGATTDummySensor

M5Stack に書き込むと GATT のサーバーとして動作し、クライアントが接続してきたら一定時間ごとに Notify するだけのプログラム (同時に1クライントまで)

あるセンサの動作を模すために作ったので Sensor という名前

## Spec

[M5Stack GREY](https://docs.m5stack.com/en/core/gray) を使ったが別のモデルでも大丈夫

1. ソースコードにハードコードされた UUID にしたがって Service およびそれに紐づく Characteristic を Advertise する

2. クライアントからの接続要求に応答して Advertise を中止し、一定時間ごとに Notification を送る

3. クライアントが切断するまで2を行い、切断後は Advertise を再開して1に戻る

Notification に載っかってクライアントに送信されるデータは整数、だんだん増える

LCD に何も表示されないのは仕様

### Bug?

たまに自分で追加したサービスが Secondary Service になってしまって Primary Service にしかアクセスできない仕様の WebBluetoothAPI から触れなくなる

使用しているライブラリの仕様なのか私の書き方が悪いのかは不明

パソコン再起動したり OS の Bluetooth 機能をオンオフしたり、なんやかんやしたら直る(かもしれない)

ここはよく分かっていないのだが他の BLE デバイスのパケットを見たりサンプルコードを読んだりした限り、このプログラムが Advertise するデータは Service のリストは載っているものの、ほぼ空らしい

そのため、 Advertise されたデータを読んで Service の有無等を判断して接続先を決定するようなクライアントだとうまく動作しない可能性がある

後述のスマホアプリではサービス名が Unknown と表示されるものの いちおう Notification を受信することはできた

## Build & Upload

[VS Code の拡張機能](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)で platformio を使うと吉

画面下部のツールバーに左の方にある右矢印アイコンをクリックするとビルドして M5Stack にアップロードできる

詳しくは[公式ドキュメント](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start)を参照されたし

なおplatformio cli がインストールされていてパスが通っているならこれでも行ける

```sh
# Build project
pio run

# Upload firmware
pio run --target upload

# Clean build files
pio run --target clean
```

:memo: VS Code の拡張機能、PlatformIO IDE for VSCode (platformio.platformio-ide) を使っていると `pio` コマンドにパスが通っていないかもしれないが実体は `~/.platformio/penv/bin` にあるはずなので直接叩くなりここをパスに追加するなりすればよい

## Debug

### View log through serial port

デバッグするときは `~/.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32/qspi_qspi/include/sdkconfig.h` の350行目あたりで定義されている`CONFIG_LOG_DEFAULT_LEVEL` の値を上げて詳細を出力させるようにするとデバッグが捗る

```c
// これを
define CONFIG_LOG_DEFAULT_LEVEL 1
// こうする
define CONFIG_LOG_DEFAULT_LEVEL 5
```

またこれに加えて以下のオプションも忘れずにコンパイラに渡す(platformio を使う分には platform.ini から自動的に読み込まれる)

```sh
-DCORE_DEBUG_LEVEL=4 # 4か5でないと詳細が出ない
```

上の手順を踏むことで吐き出されるようになるログは platformio のシリアルモニターを使って見ることができる

VS Codeなら画面下部のツールバーの左の方にあるコンセントマークをクリックする

cliは次の通り

```sh
pio device monitor -b 115200
```

ここに関しては一般的なシリアル通信なので通信速度とポートさえ正しく設定すれば Arduino IDE 等のツールでも同じことができる

### Other tools for debug

他にも自作のクライアントプログラムや　WebBluetoothAPI だとうまく通信できなかったがスマホアプリではできることもあったので、おかしいなと思ったら一度スマホアプリで様子を見るのもアリ

私は [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) を使った

あるいはパソコンをクライアントにしているなら [Wireshark](https://www.wireshark.org/) で Bluetooth アダプタがやり取りしている通信内容を眺めるのも手