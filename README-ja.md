ofxEpilog
=========

# 概要

ofxEpilogはEpilogのレーザーカッターをOpen Frameworksで使用するためのアドオンです。現在はMac OS Xのみのサポートをしており、WindowsやLinuxはサポート対象外です。
ofxEpilogの特長は、EpilogのレーザーカッターをOpen Frameworksから動的に使用できる点です。加工データをプログラム内で生成し、そのまま加工ジョブとしてレーザーカッターに送信できます。加工ジョブはレーザーカッターにTCPプロトコルで送信されます。ジョブのセッションを張り続けることでほぼリアルタイムにレーザーカッターのヘッドを操作、照射の制御できます。


インストール方法
--
* Open Frameworksがまだダウンロードされていない場合は`of_v0.8.0_osx_release.zip`をダウンロードしてください。
  今のところofxEpilogはバージョン0.8.0で開発、テストを行っています。
  `http://www.openframeworks.cc/download/`

* 最新版のofxEpilogをgithubからダウンロードしてください。
  https://github.com/mitsuhito/ofxEpilog/archive/master.zip
  ダウンロードが完了したら任意の場所に展開しましょう。

  もしくはコマンドラインがお好きな方は`git clone`をしてください。
  `git clone git://github.com/mitsuhito/ofxEpilog.git`

* ofxEpilogディレクトリを`of_v0.8.0_osx_release/addons/`ディレクトリ内にコピーしてください。
  

アドオンの使い方
--
* projectGeneratorを使用してプロジェクト新規作成にアドオンを追加する場合
  addonsの項目にofxNetworkとofxEpilogを選択し、追加してください。
  ofxEpilogAddonUsage1.png

* 既存のプロジェクトに追加する場合
  * ofxNetworkを既に使用していない場合、addonsグループの下にNew Groupでグループを新規作成し、"ofxNetwork"と名前を変更します。
  * ofxNetworkのグループの下にofxNetwork/srcをドラッグ&ドロップします。
  * Create groups for any added foldersのみチェックが入っている状態でFinishを選択します。
  * addonsグループの下にNew Groupでグループを新規作成し、"ofxEpilog"と名前を変更します。
  * ofxEpilogのグループの下にofxEpilog/srcをドラッグ&ドロップします。
  * Create groups for any added foldersのみチェックが入っている状態でFinishを選択します。
  ofxEpilogAddonUsage2.png
  ofxEpilogAddonUsage3.png

* ofxEpilogクラスの利用方法
  ヘッダファイルをインクルードします。
  ```#include "ofxEpilog.h"```

  ofxEpilogインスタンスを作成します。
  ```ofxEpilog epilogLaser;```

  使用するEpilogレーザーカッターの最大加工可能範囲を設定します。
  ```
    WorkareaSize fusion;
    fusion.width = 812;				// 横幅 mm
    fusion.height = 508;			// 高さ mm
    epilogLaser.setWorkareaSize(fusion);
  ```

  カットスピード、カットパワー、彫刻スピード、彫刻パワー、周波数、DPI、オートフォーカス設定などの加工パラメーターを設定します。
  ```
    OutputConfig config;
    config.vspeed = 80;				// カットスピード 1-100%
    config.vpower = 10;				// カットパワー   1-100%
    config.rspeed = 10;				// 彫刻スピード   1-100%
    config.rpower = 20;				// 彫刻パワー     1-100%
    config.dpi = 200;				// DPI            75-1200
    config.freq = 100;				// 周波数         1-100%
    config.autoFocusEnabled = false;		// オートフォーカス設定
    epilogLaser.setOutputConfig(config);
  ```

  レーザーカッターと接続を開始します。
  ```
    bool isConnected = epilogLaser.connect("192.168.3.5");
  ```

  接続済みの状態で加工ジョブを送信します。加工ジョブの型は`ofPolyline`, `ofImage`, `Graffiti Markup Language`をサポートしています。
  ```
    // 100mm * 100mm の矩形をカットする
    ofPolyline line;
    line.addVertex(0, 0);
    line.addVertex(100, 0);
    line.addVertex(100, 100);
    line.addVertex(0, 100);
    line.addVertex(0, 0);
    line.close();

    ofPtr<HPGLBuffer> hpglBuffer = HPGLBuffer::create(line, epilogLaser.getOutputConfig());
    
    if(epilogLaser.isConnected())
    {
        bool isSent = epilogLaser.send(hpglBuffer, VECTOR);
        ofLog(OF_LOG_NOTICE, "isSent=%d", isSent);
    }
  ```
  カスタムオブジェクトを加工ジョブとして送信したい場合、`ofPtr<HPGLBuffer> HPGLBuffer::create()`メソッドをオーバーロードするか、`HPGLBuffer`のサブクラスを作成し、ファクトリメソッドを追加することで可能になります。


  接続を切ります。
  ```
	epilogLaser.disconnect();
  ```


サンプルプログラム
--

* example-ofxEpilogBasicExample
* example-ofxEpilogGMLExample
* example-ofxEpilogLiveExample


--
mitsuhito.ando@gmail.com
