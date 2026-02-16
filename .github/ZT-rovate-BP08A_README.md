# ZT-rovate-BP08A

このリポジトリは、rovate_Bipedal_TYPE-A プロジェクトのGitHub公開用です。

## 概要
- M5Stack CoreS3用二足歩行ロボットコントローラー
- タッチUI・IMU・モーター制御・PCクライアント対応

## ディレクトリ構成
- src/ : メインソースコード
- lib/ : 外部ライブラリ
- include/ : ヘッダファイル
- tools/ : PCクライアント
- docs/ : ドキュメント

## ライセンス
MIT License

---

### 初回登録手順
1. GitHubで新規リポジトリ「ZT-rovate-BP08A」を作成
2. このプロジェクトフォルダ内で `git init` を実行
3. `.gitignore` を作成（例：buildファイルやvenv除外）
4. `git remote add origin <GitHubのURL>` を設定
5. `git add . && git commit -m "Initial commit"`
6. `git push -u origin main` でアップロード

---

### 参考
- README.md, 各サブREADME
- platformio.ini
- requirements.txt（tools/pc_client）
