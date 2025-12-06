Attribute VB_Name = "AssetCsvTools1"
Option Explicit

' == Settings ==
Private Const DATA_SHEET As String = "Data"
Private Const CONTROLS_SHEET As String = "Controls"
Private Const TABLE_NAME As String = "AssetTable"
' Data sheet columns:
'   1: type
'   2: path
'   3: aliases
'   4: scale
'   5: flip
'   6: preload
'   7: group
'   8: tags
'   9: param1
'  10: param2
'  11: notes

' --- Helpers ---

' Normalize alias: lower case, trim, remove spaces and duplicate "||"
Private Function NormalizeAlias(ByVal s As String) As String
    s = LCase$(Trim$(s))
    s = Replace$(s, " ", "")
    s = Replace$(s, vbTab, "")
    Do While InStr(s, "||") > 0
        s = Replace$(s, "||", "|")
    Loop
    NormalizeAlias = s
End Function

' Normalize path: trim, slash to "/", lower case
Private Function NormalizePath(ByVal s As String) As String
    s = Trim$(s)
    s = Replace$(s, "\", "/")
    s = LCase$(s)
    NormalizePath = s
End Function

' Get data table range
Private Function AssetRange() As Range
    Dim lo As ListObject
    On Error Resume Next
    Set lo = ThisWorkbook.Worksheets(DATA_SHEET).ListObjects(TABLE_NAME)
    On Error GoTo 0
    If lo Is Nothing Then
        Set AssetRange = Nothing
    Else
        Set AssetRange = lo.DataBodyRange
    End If
End Function

' ====== Public Macros ======

' Normalize aliases/path/tags
Public Sub Normalize()
    Dim rng As Range, r As Range
    Set rng = AssetRange()
    If rng Is Nothing Then Exit Sub

    For Each r In rng.Rows
        Dim tp As String, p As String
        Dim al As String, tg As String

        tp = CStr(r.Cells(1, 1).Value2) ' type
        p = CStr(r.Cells(1, 2).Value2)  ' path
        al = CStr(r.Cells(1, 3).Value2) ' aliases
        tg = CStr(r.Cells(1, 8).Value2) ' tags

        If Len(al) > 0 Then
            r.Cells(1, 3).Value = NormalizeAlias(al)
        End If

        If Len(p) > 0 Then
            r.Cells(1, 2).Value = NormalizePath(p)
        End If

        If Len(tg) > 0 Then
            ' tags are normalized similar to aliases
            r.Cells(1, 8).Value = NormalizeAlias(tg)
        End If
    Next r
End Sub

' Check duplicate aliases in whole table
Public Sub DeduplicateAliasesReport()
    Dim rng As Range, r As Range
    Set rng = AssetRange()
    If rng Is Nothing Then Exit Sub

    Dim dict As Object
    Set dict = CreateObject("Scripting.Dictionary")

    Dim msg As String
    msg = ""

    For Each r In rng.Rows
        Dim aliasStr As String
        aliasStr = CStr(r.Cells(1, 3).Value2) ' aliases

        If Len(aliasStr) > 0 Then
            Dim parts() As String
            parts = Split(aliasStr, "|")
            Dim i As Long
            For i = LBound(parts) To UBound(parts)
                Dim a As String
                a = NormalizeAlias(parts(i))
                If Len(a) > 0 Then
                    If dict.Exists(a) Then
                        msg = msg & vbCrLf & _
                              a & " : row " & CStr(dict(a)) & " and row " & CStr(r.Row)
                    Else
                        dict.Add a, r.Row
                    End If
                End If
            Next i
        End If
    Next r

    If Len(msg) = 0 Then
        MsgBox "No duplicate aliases.", vbInformation
    Else
        MsgBox "Duplicate aliases found:" & msg, vbExclamation
    End If
End Sub

' Export Data table to CSV
Public Sub ExportCsv()
    Dim ws As Worksheet: Set ws = ThisWorkbook.Worksheets(DATA_SHEET)
    Dim lo As ListObject: Set lo = ws.ListObjects(TABLE_NAME)
    If lo Is Nothing Then
        MsgBox "Table not found: " & TABLE_NAME, vbCritical
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
        outPath = Application.GetSaveAsFilename( _
                    InitialFileName:="Data.csv", _
                    FileFilter:="CSV (*.csv),*.csv")
    End If

    If VarType(outPath) = vbBoolean Then Exit Sub

    Dim fnum As Integer: fnum = FreeFile
    Open CStr(outPath) For Output As #fnum

    Dim r As Range, i As Long
    For Each r In lo.DataBodyRange.Rows
        Dim vals(1 To 11) As String
        For i = 1 To 11
            Dim v As Variant: v = r.Cells(1, i).Value
            If IsError(v) Then v = ""
            vals(i) = CStr(v)
        Next i

        ' skip completely empty row
        If (Len(Trim$(vals(1))) + Len(Trim$(vals(2))) + Len(Trim$(vals(3)))) > 0 Then
            Print #fnum, Join(vals, ",")
        End If
    Next r

    Close #fnum
    MsgBox "CSV export completed: " & CStr(outPath), vbInformation
End Sub

' Import CSV to Data table (overwrite current rows)
Public Sub ImportCsv()
    Dim inPath As Variant
    inPath = Application.GetOpenFilename(FileFilter:="CSV (*.csv),*.csv")
    If VarType(inPath) = vbBoolean Then Exit Sub

    Dim ws As Worksheet: Set ws = ThisWorkbook.Worksheets(DATA_SHEET)
    Dim lo As ListObject: Set lo = ws.ListObjects(TABLE_NAME)
    If lo Is Nothing Then
        MsgBox "Table not found: " & TABLE_NAME, vbCritical
        Exit Sub
    End If

    ' Clear existing rows
    On Error Resume Next
    Do While lo.ListRows.Count > 0
        lo.ListRows(lo.ListRows.Count).Delete
    Loop
    On Error GoTo 0

    Dim fnum As Integer: fnum = FreeFile
    Open CStr(inPath) For Input As #fnum

    Dim line As String
    Do While Not EOF(fnum)
        Line Input #fnum, line
        line = Trim$(line)
        If Len(line) = 0 Then GoTo NextLine

        Dim parts() As String
        parts = Split(line, ",")

        ' Support both old 6 columns and new 11 columns
        If UBound(parts) < 10 Then
            ReDim Preserve parts(0 To 10)
        End If

        Dim newRow As ListRow
        Set newRow = lo.ListRows.Add
        Dim i As Long
        For i = 0 To 10
            newRow.Range.Cells(1, i + 1).Value = parts(i)
        Next i
NextLine:
    Loop

    Close #fnum
    MsgBox "CSV import completed.", vbInformation
End Sub
