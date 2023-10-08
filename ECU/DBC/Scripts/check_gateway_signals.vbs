''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' This VB-Script checks the gateway signals of a control unit in a
' CANdb++ database (MDC-File)
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

Dim args, filename, filetype, gateway, gatewayid, gwattrid
Dim con, recset1, recset2

'Parameter auswerten und ggf nachfragen
Set args = WScript.Arguments

if args.count = 0 then 
  filename = inputbox("Please enter database filename, allowed types *.mdc","CANdb++")  
else
  filename = args(0)
end if

if not valid(filename) then
  call msgbox("Invalid CANdb++ file name, this script is for MDC-files only",0,"CANdb++")
  wscript.quit 5
end if

if args.count <= 1 then
  gateway = ""
else
  gateway = args(1)
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


set recset1 = createobject("ADODB.recordset")
set recset2 = createobject("ADODB.recordset")

'get the id for the gateway attribute
recset1.open "SELECT DB_ID FROM  Cdb_Attribute_Definition WHERE Name LIKE 'Gatewaysignal'",con
if recset1.eof then
  call msgbox("There is no attribute definition for gateway signals",0,"CANdb++")
  wscript.quit 5
end if
gwattrid = recset1(0).value
recset1.close

'Get the gateway id 
gatewayid = null
recset1.open "SELECT DB_ID FROM Cdb_ECU WHERE Name LIKE '" & gateway & "'",con
if recset1.eof then
  recset1.close
  gateway = inputbox("Please enter a valid ECU Name","CANdb++")  
  recset1.open "SELECT DB_ID FROM Cdb_ECU WHERE Name LIKE '" & gateway & "'",con
  if recset1.eof then
    call msgbox("Gateway " & gateway & " not found",0,"CANdb++")
    recset1.close
    con.close
    wscript.quit 5
  end if
end if
gatewayid = recset1(0).value
recset1.close

'all rx-signals of the given gateway
recset2.open "SELECT * FROM Cdb_Attribute_Value WHERE " &_ 
               "Object_ID IN (SELECT Signal FROM Cdb_Node_RxSignal WHERE " &_
               "Node IN (SELECT Node FROM Cdb_ECU_Node WHERE " &_
               "ECU =" & cstr(gatewayid)  & ")) " &_
             "AND " &_
               "Attribute_Definition = " & gwattrid,con 

'all tx-signals of the given gateway
recset1.open "SELECT * FROM Cdb_Attribute_Value WHERE " &_
               "Object_ID IN (SELECT DB_ID FROM Cdb_Signal WHERE " &_
               "DB_ID IN (SELECT Signal FROM Cdb_Message_Signal WHERE " &_
               "Message IN (SELECT DB_ID FROM Cdb_Message WHERE " &_
               "Sender IN (SELECT Node FROM Cdb_ECU_Node WHERE " &_
               "ECU =" & gatewayid & ")))) " &_
             "AND " &_
               "Attribute_Definition = " & gwattrid,con




Dim foundtx, corrsig(), norx(), notx(), finalmessage, foundrx
dim rec2null, rec3null    

finalmessage = ""
redim preserve corrsig(1)
corrsig(0) = 0
corrsig(1) = 0
redim preserve notx(0)
notx(0) = 0
redim preserve norx(0)
norx(0) = 0
rec2null = false
if recset2.eof then
  rec2null = true
end if
rec3null = false
if recset1.eof then
  rec3null = true
end if

'look if there is a corresponding tx-signal for each rx-signal
'if there is one, remember the two signals in corrsig-array
do while not recset2.eof
  foundtx = 0
  if not rec3null then
    do while not recset1.eof
      if recset1(3).value = recset2(3).value then
        foundtx = foundtx + 1    
        corrsig(ubound(corrsig)-1) = recset2(0).value
        corrsig(ubound(corrsig)) = recset1(0).value
        redim preserve corrsig(ubound(corrsig)+2)
      end if
      recset1.movenext
    loop 
    recset1.movefirst
  end if
  if foundtx = 0 then 
    notx(ubound(notx)) = recset2(0).value
    redim preserve notx(ubound(notx)+1)     
  end if
  recset2.movenext
loop

'recset2 wieder auf ersten eintrag setzen, falls einer ueberhaupt existiert
if not rec2null then
  recset2.movefirst
end if

'check if there is a rx-signal for each tx-signal
do while not recset1.eof
  foundrx = 0
  if not rec2null then
    do while not recset2.eof
      if recset2(3).value = recset1(3).value then
        foundrx = foundrx + 1
      end if
      recset2.movenext
    loop
    recset2.movefirst
  end if
  if foundrx = 0 then 
    norx(ubound(norx)) = recset1(0).value 
    redim preserve norx(ubound(norx)+1)
  end if
  recset1.movenext
loop 

recset2.close
recset1.close

Dim sigindex
'output all rx-signals without tx-signal
for sigindex = 0 to ubound(notx)-1
  recset1.open "SELECT Name FROM Cdb_Signal WHERE DB_ID = " & notx(sigindex)
  finalmessage = finalmessage &  "RX-Signal " & recset1(0).value &_
                                 " has no TX-signal" & chr(13)
  recset1.close
next

'output all tx-signals without rx-signal
for sigindex = 0 to ubound(norx)-1
  recset1.open "SELECT Name FROM Cdb_Signal WHERE DB_ID = " & norx(sigindex)
  finalmessage = finalmessage &  "TX-Signal " & recset1(0).value &_ 
                                 " has no RX-signal" & chr(13)
  recset1.close
next

'check if corresponding signals differ in length, max, min ...
Dim diff
sigindex = 0
do while sigindex <=  (ubound(corrsig)-2)
  recset1.open "SELECT * FROM Cdb_Signal WHERE DB_ID = " & cstr(corrsig(sigindex)),con
  sigindex = sigindex + 1
  recset2.open "SELECT * FROM Cdb_Signal WHERE DB_ID = " & cstr(corrsig(sigindex)),con
  sigindex = sigindex + 1 
  diff = sigdiff(recset1,recset2)
  if diff <> "" then
  finalmessage = finalmessage & "Signals " & recset1(1).value & " and " & recset2(1).value  & " differ in " & diff & chr(13)
  end if  
  recset1.close
  recset2.close
loop


if finalmessage = "" then
  call msgbox("All gatewaysignals OK",0,"CANdb++")
else
  call msgbox(finalmessage,0,"CANdb++")
end if
con.close


'this function returns the differences of two signals as a string
'the signals are given as records
function sigdiff(sig1,sig2)
  sigdiff = ""
  if sig1(2).value <> sig2(2).value then
    sigdiff = sigdiff & "signal length, "
  end if
  if (sig1(6).value <> sig2(6).value) or (sig1(7).value <> sig2(7).value) then
    sigdiff = sigdiff & "formula, "
  end if
  if sig1(8).value <> sig2(8).value then
    sigdiff = sigdiff & "minimum, "
  end if
  if sig1(9).value <> sig2(9).value then
    sigdiff = sigdiff & "maximum, "
  end if
  if sig1(10).value <> sig2(10).value then
    sigdiff = sigdiff & "unit, "
  end if
  if sigdiff <> "" then
    sigdiff = left(sigdiff,len(sigdiff)-2)  
  end if
end function 

'checks wether the input-file has an mdc-extension
function valid(name)
  if (instr(name,".mdc") = len(name) - 3) then
    valid = true
  else 
    valid = false
  end if
end function


