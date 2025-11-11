
Attribute VB_Name = "AssetCsvTools"
Option Explicit

' == 設定 ==
Private Const DATA_SHEET As String = "Data"
Private Const CONTROLS_SHEET As String = "Controls"
Private Const TABLE_NAME As String = "AssetTable"

' 文字正規化ヘルパー
Private Function NormalizeAlias(ByVal s As String) As String
    s = LCase$(Trim$(s))
    s = Replace$(s, " ", "")
    s = Replace$(s, vbTab, "")
    ' 余分な区切り "||" を単一に
    Do While InStr(s, "||") > 0
        s = Replace$(s, "||", "|")
    Loop
    NormalizeAlias = s
End Function

Private Function NormalizePath(ByVal s As String) As String
    s = Trim$(s)
    s = Replace$(s, "\", "/")
    s = LCase$(s)
    NormalizePath = s
End Function

' Data シートのテーブル範囲を返す
Private Function AssetRange() As Range
    Dim lo As ListObject
    Set lo = ThisWorkbook.Worksheets(DATA_SHEET).ListObjects(TABLE_NAME)
    Set AssetRange = lo.DataBodyRange
End Function

' ====== Public Macros ======

' 入力の正規化：aliases/path の体裁を整える
Public Sub Normalize()
    Dim rng As Range, r As Range
    On Error GoTo ExitNormalize
    Set rng = AssetRange()
    If rng Is Nothing Then Exit Sub

    For Each r In rng.Rows
        Dim tp As String, p As String, al As String, sc As String, fl As String, pw As String
        tp = CStr(r.Cells(1, 1).Value2) ' type
        p  = CStr(r.Cells(1, 2).Value2) ' path
        al = CStr(r.Cells(1, 3).Value2) ' aliases

        If Len(al) > 0 Then r.Cells(1, 3).Value = NormalizeAlias(al)
        If Len(p)  > 0 Then r.Cells(1, 2).Value = NormalizePath(p)
    Next r

    MsgBox "Normalize 完了", vbInformation
ExitNormalize:
End Sub

' aliases の重複を検出し、レポートする（簡易）
Public Sub DeduplicateAliasesReport()
    Dim dict As Object: Set dict = CreateObject("Scripting.Dictionary")
    Dim rng As Range, r As Range, al As String, i As Long
    Set rng = AssetRange()
    If rng Is Nothing Then Exit Sub

    Dim msg As String: msg = ""
    For Each r In rng.Rows
        al = CStr(r.Cells(1, 3).Value2)
        If Len(al) > 0 Then
            Dim parts() As String
            parts = Split(al, "|")
            For i = LBound(parts) To UBound(parts)
                Dim key As String: key = Trim$(parts(i))
                If Len(key) = 0 Then GoTo NextPart
                If dict.Exists(key) Then
                    msg = msg & vbCrLf & "Duplicate: " & key & " (row " & r.Row & " and row " & dict(key) & ")"
                Else
                    dict.Add key, r.Row
                End If
NextPart:
            Next i
        End If
    Next r

    If Len(msg) = 0 Then
        MsgBox "重複なし！", vbInformation
    Else
        MsgBox "重複検出:" & msg, vbExclamation
    End If
End Sub

' Data テーブルを CSV 書き出し
Public Sub ExportCsv()
    Dim ws As Worksheet: Set ws = ThisWorkbook.Worksheets(DATA_SHEET)
    Dim lo As ListObject: Set lo = ws.ListObjects(TABLE_NAME)
    If lo Is Nothing Then
        MsgBox "テーブルが見つかりません: " & TABLE_NAME, vbCritical
        Exit Sub
    End If

    Dim pathCell As Range
    On Error Resume Next
    Set pathCell = ThisWorkbook.Worksheets(CONTROLS_SHEET).Range("B8")
    On Error GoTo 0

    Dim outPath As Variant
    If Not pathCell Is Nothing And Len(CStr(pathCell.Value)) > 0 Then
        outPath = CStr(pathCell.Value)
    Else
        outPath = Application.GetSaveAsFilename(InitialFileName:="Data.csv", FileFilter:="CSV (*.csv),*.csv")
    End If
    If VarType(outPath) = vbBoolean Then Exit Sub

    Dim fnum As Integer: fnum = FreeFile
    Open CStr(outPath) For Output As #fnum

    Dim r As Range, i As Long
    For Each r In lo.DataBodyRange.Rows
        Dim vals(1 To 6) As String
        For i = 1 To 6
            Dim v As Variant: v = r.Cells(1, i).Value
            If IsError(v) Then v = ""
            vals(i) = CStr(v)
        Next i
        ' 空行はスキップ（type/path/aliases どれも空なら）
        If (Len(Trim$(vals(1))) + Len(Trim$(vals(2))) + Len(Trim$(vals(3)))) > 0 Then
            Print #fnum, Join(vals, ",")
        End If
    Next r

    Close #fnum
    MsgBox "CSV 出力完了: " & CStr(outPath), vbInformation
End Sub

' CSV 読み込み（現在の Data を上書き）
Public Sub ImportCsv()
    Dim inPath As Variant
    inPath = Application.GetOpenFilename(FileFilter:="CSV (*.csv),*.csv")
    If VarType(inPath) = vbBoolean Then Exit Sub

    Dim ws As Worksheet: Set ws = ThisWorkbook.Worksheets(DATA_SHEET)
    Dim lo As ListObject: Set lo = ws.ListObjects(TABLE_NAME)

    ' 既存の行をクリア
    On Error Resume Next
    lo.DataBodyRange.Rows.Delete
    On Error GoTo 0

    Dim fnum As Integer: fnum = FreeFile
    Open CStr(inPath) For Input As #fnum

    Dim line As String, parts() As String
    Do While Not EOF(fnum)
        Line Input #fnum, line
        If Len(line) = 0 Then GoTo NextLine
        parts = Split(line, ",")
        ' 6列に満たない場合は埋める
        ReDim Preserve parts(0 To 5)
        Dim newRow As ListRow
        Set newRow = lo.ListRows.Add
        Dim i As Long
        For i = 0 To 5
            newRow.Range.Cells(1, i + 1).Value = parts(i)
        Next i
NextLine:
    Loop
    Close #fnum

    MsgBox "CSV 読み込み完了", vbInformation
End Sub
