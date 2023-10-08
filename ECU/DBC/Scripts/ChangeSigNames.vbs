' This VB-Script changes the names of signals in the database
'------------------------------------------------------------
' Copyright (c) 2000 Vector Informatik GmbH.  All rights reserved.

Option Explicit

Dim cnn, args, filename, filetype

Set args = WScript.Arguments

if args.count = 0 then
  filename = inputbox("Please enter database filename, allowed types *.mdc *.dbc","CANdb++")  
else 
  filename = args(0)
end if

if not valid(filename) then
  call msgbox("Invalid CANdb++ file name",0,"CANdb++")
  wscript.quit 1
end if

filetype = gettype(filename)

' Create an ADO connection object and open the database  
set cnn = createobject("ADODB.Connection")
if filetype = "mdc" then
  cnn.connectionstring = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" & filename 
else
  cnn.ConnectionString = "Provider=CanDbPrv.CANdb.1;Data Source=" & filename
end if

on error resume next
cnn.open
if not err.description = "" then 
  call msgbox (filename & " is not a valid CANdb++ file",0,"CANdb++")
  err.clear
  wscript.quit 1
end if
on error goto 0

if filetype = "dbc" then
  cnn.BeginTrans
end if

' open the rowset for signals
Dim rsSignal
Set rsSignal = CreateObject("ADODB.Recordset")
rsSignal.Open "Cdb_Signal", cnn

' loop over all signals and append _CAN to the name
Dim newSignalName
do while not rsSignal.eof
  newSignalName = rsSignal("Name")
  newSignalName = newSignalName + "_CAN"
'  newSignalName = replace(newSignalName, "CAN", "LIN")
  SetSignalName rsSignal, newSignalName
  rsSignal.MoveNext
loop

' Close the connection
If filetype = "dbc" Then
  cnn.CommitTrans
End If
cnn.Close

' Display the results
MsgBox("Names of Signals succescfully changed")
wscript.quit 2

' function to update the current signal (rowset object)
' -----------------------------------------------------
function SetSignalName(rsSignal, name)
  Dim fieldNames(11)
  Dim values(11)
  fieldNames(0) = "DB_ID"
  fieldNames(1) = "Name"
  fieldNames(2) = "Bitsize"
  fieldNames(3) = "Byteorder"
  fieldNames(4) = "Valuetype"
  fieldNames(5) = "Initialvalue"
  fieldNames(6) = "Formula_Factor"
  fieldNames(7) = "Formula_Offset"
  fieldNames(8) = "Minimum"
  fieldNames(9) = "Maximum"
  fieldNames(10) = "Unit"
  fieldNames(11) = "Comment"
  
  values(0) = rsSignal("DB_ID")
  values(1) = name
  values(2) = rsSignal("Bitsize")
  values(3) = rsSignal("Byteorder")
  values(4) = rsSignal("Valuetype")
  values(5) = rsSignal("Initialvalue")
  values(6) = rsSignal("Formula_Factor")
  values(7) = rsSignal("Formula_Offset")
  values(8) = rsSignal("Minimum")
  values(9) = rsSignal("Maximum")
  values(10) = rsSignal("Unit")
  values(11) = rsSignal("Comment")
  
  If (filetype = "mdc") Then
    Dim sqlString
    sqlString = "UPDATE Cdb_Signal SET Name = '" & name & _
                    "' WHERE DB_ID = " & rsSignal("DB_ID")
    cnn.Execute (sqlString)
  else
    rsSignal.Update fieldNames, values
  end if
end function

' function to validate the file name
' ----------------------------------
function valid(name)
  if (instr(name,".mdc") = len(name) - 3) or (instr(name,".dbc") = len(name) - 3) then
    valid = true
  else 
    valid = false
  end if
end function

' function to retrieve the type of the datebase file
' --------------------------------------------------
function gettype(name)
  if instr(name,".mdc") = len(name) - 3 then
    gettype = "mdc"
  else
    gettype = "dbc"
  end if
end function


