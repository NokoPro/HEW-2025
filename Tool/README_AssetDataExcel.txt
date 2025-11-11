Asset Data.csv エディタ（Excel）使い方
=====================================

同梱ファイル:
- AssetDataEditor.xlsx : 編集用テンプレート（検証・色分け付き）
- AssetCsvTools.bas : マクロ（VBA）モジュール

セットアップ:
1) Excelで AssetDataEditor.xlsx を開く
2) [開発]タブ > [Visual Basic] > VBA画面で、プロジェクトを右クリック -> [ファイルのインポート]
3) 'AssetCsvTools.bas' を選択してインポート
4) ファイル形式を [Excel マクロ有効ブック (*.xlsm)] で保存（例: AssetDataEditor.xlsm）

使い方:
- Data シートに行を追加して編集。バリデーションと色分けでエラーを可視化します。
- Controls シート B8 に CSV の出力先パスを書いておくと、ExportCsv でダイアログなし保存できます。
- マクロ:
    - Normalize: 体裁を整えます（aliases 小文字化/空白除去、path の \ を / に置換）
    - DeduplicateAliasesReport: 重複 alias を簡易レポート
    - ExportCsv: Data を CSV に出力
    - ImportCsv: CSV から Data を読み込み

推奨ルール:
- aliases は小文字+アンダースコア、種別プレフィックスを付ける（mdl_, tex_ など）
- path は Assets/ からの相対・小文字・スラッシュ・拡張子必須
