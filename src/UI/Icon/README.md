## Icon - アイコン描画（簡潔）

用途
- 小さな矢印やラベル付きのアイコンを描画するユーティリティです。主に UI の装飾に使います。

使い方（最小限）
- `#include "UI/Icon/Icon.h"`
- `icon.drawArrow(x,y,length,color,"up")` または `icon.drawCommonIcon(label,x,y,w,h,r,color)`
- 描画は `M5.Canvas` 上で行い、最後に `M5.Canvas.pushSprite(0,0)` で反映します。

注意
- アイコンは描画専用で、タッチ処理は行いません。必要ならボタンなどと組み合わせてください。
