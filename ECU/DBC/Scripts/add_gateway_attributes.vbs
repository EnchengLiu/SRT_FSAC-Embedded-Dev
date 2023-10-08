''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' This VB-Script insert user defined attributes to check
' gateway signal in a CANdb++ database (MDC-File)
'
' Gateway signals must have the value for attribute 'Gatewaysignal'
' (case sensitive) set. An RX-Signal and a TX-Signal representing a
' gateway signal in a control unit, must have the same value for this
' attribute.
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' Copyright (c) 2000 Vector Informatik GmbH.  All rights reserved.
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

option explicit

Dim args, con, filename, recset, attrid, columnid

attrid = 1
columnid = 0

'Parameter auswerten und ggf nachfragen
Set args = WScript.Arguments

if args.count = 0 then 
  filename = inputbox("Please enter database filename, allowed types *.mdc","CANdb++")  
else
  filename = args(0)
end if

if not valid(filename) then
  if filetype(filename) = "dbc" then
    call msgbox("This Script doesn't support dbc-files")    
  else
    call msgbox("Invalid CANdb++ file name",0,"CANdb++")
  end if
  wscript.quit 5
end if

'Datei oeffnen
set con = createobject("ADODB.Connection")
con.connectionstring = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" & filename 
on error resume next
con.open
if not err.description = "" then 
  call msgbox (filename & " is not a valid CANdb++ file",0,"CANdb++")
  err.clear
  wscript.quit 5  
end if
on error goto 0

set recset = createobject("ADODB.recordset")


set recset = con.execute("SELECT * FROM Cdb_Attribute_Definition")
if not recset.eof then
' Get next database ID
  set recset = con.execute("SELECT max(DB_ID) FROM Cdb_Attribute_Definition")
  set attrid = recset("Expr1000")
  attrid = attrid + 1
' Get next column index
  set recset = con.execute("SELECT max(Column_Index) FROM Cdb_Attribute_Definition")
  columnid = recset("Expr1000")
  columnid = columnid + 1
end if

'add attribute Gateway if it doesn't exist
set recset = con.execute("SELECT * FROM Cdb_Attribute_Definition WHERE Name LIKE 'Gatewaysignal'")
if recset.eof then
  Set recset = con.execute("INSERT INTO Cdb_Attribute_Definition VALUES (" & attrid  & ", 'Gatewaysignal', 10, 2, 0, 0, '', 0, ''," & columnId & ", '')")
end if
wscript.quit 2

function filetype(name)
  if (instr(name,".mdc") = len(name) - 3) then
    filetype = "mdc"
  end if
  if (instr(name,".dbc") = len(name) - 3) then
    filetype = "dbc"
  else 
    filetype = ""
  end if
end function

function valid(name)
  if (instr(name,".mdc") = len(name) - 3) then
    valid = true
  else 
    valid = false
  end if
end function

