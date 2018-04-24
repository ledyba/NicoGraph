# NicoGraph
NicoGraphは、ニコニコ動画のタグを階層的にまとめあげた上で可視化するWebアプリケーションです。

超高速なグラフクラスタリングアルゴリズムを用いており、その高速性を活かして集計期間をダイナミックに変更してその場でクラスタリング・可視化をしてしまいます。

![ScreenShot](https://raw.githubusercontent.com/ledyba/NicoGraph/master/screenshot.jpg)

# ドキュメント

 - ニコニコ動画のタグネットワークをリアルタイムに可視化する
   - https://7io.org/2015/04/25/23:01:34/

# Louvain Method
クラスタリングに用いているアルゴリズムの実装はこちらにあります。

[ledyba/cpp-louvain-fast](https://github.com/ledyba/cpp-louvain-fast)
