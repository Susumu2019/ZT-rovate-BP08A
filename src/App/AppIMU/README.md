## AppInfo - アプリ情報表示

### 🎯 役割
アプリのバージョン、作成者、説明などを表示する「アバウト画面」です。

### 📁 ファイル
- `AppInfo.h` - クラス定義
- `AppInfo.cpp` - 情報表示の実装

### 使い方
```cpp
// AppManagerに登録するだけ
appManager.registerApp(new AppInfo());
```

ホーム画面から「Info」アイコンをタップすると表示されます。

### 💡 カスタマイズ
`AppInfo.cpp` の `draw()` メソッドを編集して、表示する情報を変更できます:
```cpp
void AppInfo::draw(M5Canvas &canvas) {
  canvas.drawString("My Project v1.0", 50, 100);
  canvas.drawString("Author: YourName", 50, 130);
}
```
