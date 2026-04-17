
# MCAs

python driver for AMPTEK MCA8000D and KROMEK K102 DAQ
original driver: https://github.com/HenningFo/mca8000d, https://github.com/rustam-lantern/LanternSpectrometer　<img width="838" height="70" alt="image" src="https://github.com/user-attachments/assets/a08f9396-c3bd-4d40-be15-b34dcc8a54be" />


## データ取得
### $ runMCA.py　　
最新のper* を作成してそこにデータを取得

データ取得中にsを押すと今取っているfileを取り終わって終了

config fileを編集することで設定変更可能

defaultでは60秒ごとに1file作る。defaultではSN???_?.mcaというfileを作って行く。

## 解析
### /MCAs/root_macros/DrawMCA.C　を編集してみたいfileを見る。
### $ root ‘~/MCAs/root_macros/DrawMCAs.C(“SN???”,#)’ 
現在のdirectoryでSN???_*.mca というfileの#個分のfileを足し上げる　結果はSN???.root　として保存される。
### $ root '~/MCAs/root_macros/hist_comp.C("ROOTFILE1","ROOTFILE2")‘
　　ROOTFILE1　と　ROOTFILE2を比較できる

  
