ofxEpilog
=========

# 概要
ofxEpilog は Epilog のレーザーカッターを Open Frameworks で使用するためのアドオンです。現在は Mac OS X のみのサポートをしており、 Windows や Linux はサポート対象外です。
ofxEpilog の特長は、Epilog のレーザーカッターを Open Frameworks から動的に使用できる点です。加工データをプログラム内で生成し、そのまま加工ジョブとしてレーザーカッターに送信できます。加工ジョブはレーザーカッターに TCP プロトコルで送信されます。ジョブのセッションを張り続けることでほぼリアルタイムにレーザーカッターのヘッドを操作、照射の制御ができます。


インストール方法
---
* Open Frameworks がまだダウンロードされていない場合は `of_v0.8.0_osx_release.zip` をダウンロードしてください。
  現在の所、ofxEpilog はバージョン0.8.0で開発とテストを行っています。  
  **<http://www.openframeworks.cc/download/>**

* 最新版の ofxEpilog を github からダウンロードしてください。ダウンロードが完了したら任意の場所に展開しましょう。  
  **<https://github.com/YCAMInterlab/ofxEpilog/archive/master.zip>**
  
  もしくはコマンドラインがお好きな方は `git clone git://github.com/YCAMInterlab/ofxEpilog.git` をしてください。  

* ofxEpilog ディレクトリを `of_v0.8.0_osx_release/addons/` ディレクトリ内にコピーしてください。
  

ofxEpilog の使い方
---
* __projectGenerator を使用してプロジェクト新規作成にアドオンを追加する場合__  
    addons の項目に ofxNetwork と ofxEpilog を選択し、追加してください。  
    ![projectGenerator screen shot](https://raw.github.com/YCAMInterlab/ofxEpilog/master/images/ofxEpilogAddonUsage1.png)

* __既存のプロジェクトに追加する場合__
    * ofxNetwork を既に使用していない場合、addons グループの下に New Group でグループを新規作成し、"ofxNetwork" と名前を変更します。
    * ofxNetwork のグループの下に ofxNetwork/src をドラッグ&ドロップします。
    * Create groups for any added folders のみチェックが入っている状態で Finish を選択します。
    * addons グループの下に New Group でグループを新規作成し、"ofxEpilog" と名前を変更します。
    * ofxEpilog のグループの下に ofxEpilog/src をドラッグ&ドロップします。
    * Create groups for any added folders のみチェックが入っている状態で Finish を選択します。  
    <img src="https://raw.github.com/YCAMInterlab/ofxEpilog/master/images/ofxEpilogAddonUsage3.png"/>


* __ofxEpilog クラスの利用方法__  

    * ヘッダファイルをインクルードします。
    ```cpp
    #include "ofxEpilog.h"
    ```  

    * ofxEpilog インスタンスを作成します。
    ```cpp
    ofxEpilog epilogLaser;
    ```  

    * 使用する Epilog レーザーカッターの最大加工可能範囲を設定します。
    ```cpp
    WorkareaSize fusion;
    fusion.width = 812;				    		// 横幅 mm
    fusion.height = 508;			    		// 高さ mm
    epilogLaser.setWorkareaSize(fusion);
    ```  

    * カットスピード、カットパワー、彫刻スピード、彫刻パワー、周波数、DPI、オートフォーカス設定などの加工パラメーターを設定します。
    ```cpp
    OutputConfig config;
    config.vspeed = 80;				    		// カットスピード 1-100%
    config.vpower = 10;				    		// カットパワー   1-100%
    config.rspeed = 10;				    		// 彫刻スピード   1-100%
    config.rpower = 20;				    		// 彫刻パワー     1-100%
    config.dpi = 200;				        	// DPI            75-1200
    config.freq = 100;				        	// 周波数         1-100%
    config.autoFocusEnabled = false;				// オートフォーカス設定
    epilogLaser.setOutputConfig(config);
    ```  

    * レーザーカッターに割り当てられたIPアドレスを使用して接続を開始します。
    ```cpp
    bool isConnected = epilogLaser.connect("192.168.3.5");
    ```  

    * 接続済みの状態で加工ジョブを送信します。加工ジョブの型は `ofPolyline` , `ofImage` , `GMLBuffer (Graffiti Markup Language)` をサポートしています。
    ```cpp
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

    _カスタムオブジェクトを加工ジョブとして送信したい場合、 `ofPtr<HPGLBuffer> HPGLBuffer::create()` メソッドをオーバーロードするか、 `HPGLBuffer` のサブクラスを作成し、ファクトリメソッドを追加することで可能になります。_  

    * 接続を切ります。    
    ```cpp
    epilogLaser.disconnect();
    ```  

サンプルプログラム
---
* example-ofxEpilogBasicExample
* example-ofxEpilogGMLExample
* example-ofxEpilogLiveExample


