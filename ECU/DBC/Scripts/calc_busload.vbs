''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' This VB-Script calculates the busload of a specified network
'
' The baudrate and the message cycle times are read out of the database
' (only available in CANdb++ databases (MDC files).
' A default baudrate of 500000 Baud and a default cycletime of 100 ms are
' assumed .
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' Copyright (c) 2000 Vector Informatik GmbH.  All rights reserved.
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''


Option Explicit

const defaultzyklus = 1000
const defaultbaudrate = 500000

Dim conobj, args, buslast, numbits, baudrate, network_id
Dim filename, networkname, recset, filetype

' Read in command line arguments. Ask for a database if no arguments provided.
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

if args.count <= 1 then
  networkname = ""
else
  networkname = args(1)
end if
  

set conobj = createobject("ADODB.Connection")
if filetype = "mdc" then
  conobj.connectionstring = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" & filename 
else
  conobj.ConnectionString = "Provider=CanDbPrv.CANdb.1;Data Source=" & filename
end if

on error resume next
conobj.open
if not err.description = "" then 
  call msgbox (filename & " is not a valid CANdb++ file",0,"CANdb++")
  err.clear
  wscript.quit 1  
end if
on error goto 0


set recset = CreateObject("ADODB.Recordset")

'Get the network_id
if filetype = "mdc" then
  recset.open "SELECT * FROM Cdb_Network WHERE Name LIKE '" & networkname & "'",conobj
  if recset.eof then
    recset.close
    networkname = inputbox("Please enter a valid network name","CANdb++")    
    recset.open "SELECT * FROM Cdb_Network WHERE Name LIKE '" & networkname & "'",conobj		     
    if recset.eof then
      call Msgbox("Network Name " & networkname & " not found",0,"CANdb++")
      recset.close
      conobj.close		
      WScript.Quit 1
    end if
  end if
else
  recset.open "Cdb_Network",conobj
end if

network_id = recset(0).value
baudrate = recset(4).value
if isnull(baudrate) then
  baudrate = defaultbaudrate
elseif baudrate = 0 then 
  baudrate = defaultbaudrate
end if
recset.close

numbits = get_numbits(conobj, network_id, filetype)
conobj.close
buslast = (100 * numbits)/baudrate
call msgbox("There is a bus load of " & cstr(buslast) & "%",0,"CANdb++")
wscript.quit 0


function get_numbits(db_con, network_id, ftype)
  Dim result, network_grp_id(), recset, messageset1, reslength, grpidlength
    
  result = 0
  set recset = createobject("ADODB.recordset") 
  set messageset1 = createobject("ADODB.recordset") 
  messageset1.open "Cdb_Message",conobj

  'Knoten, die direkt im Netzwerk sind holen
  recset.open "Cdb_Network_Node", db_con
  do while not recset.eof 
    if recset(0).value = network_id then
      result = result + add_node(messageset1,recset,1) 
    end if
    recset.movenext
  loop
  recset.close  

  if ftype = "mdc" then
    'Retrieve nodes out of nodegroups 
    'first retrieve the nodegroups 
    recset.open "SELECT Object_DB_ID FROM Cdb_Group_Object WHERE Parent_Type = 2 AND Object_Type = 14 AND Parent_DB_ID = " & cstr(network_id), db_con
    do while not recset.eof
      redim network_grp_id(grpidlength)
      network_grp_id(grpidlength) = recset(0).value
      grpidlength = grpidlength + 1
      recset.movenext
    loop
    recset.close   
   
   'than retrieve the nodes of each nodegroup
    do while grpidlength > 0
      recset.open "SELECT Object_DB_ID FROM Cdb_Group_Object WHERE Parent_Type = 14 AND Object_Type = 6 AND Parent_DB_ID = " & cstr(network_grp_id(grpidlength - 1)), db_con
      do while not recset.eof
        result = result + add_node(messageset1,recset,0)
        recset.movenext
      loop
      grpidlength = grpidlength - 1                 
      recset.close  
    loop   
  end if
  
  get_numbits = result

end function

'if recset(index).value is transmitter in messet, retrieve and return the busload
function add_node(messet, recset, index)
  add_node = 0
  messet.movefirst
  do while not messet.eof
    if messet(7).value = recset(index).value then
      add_node = add_node + add_this_message(messet) 
    end if
    messet.movenext   
  loop
end function 

'calculate the busload for messages in messet
function add_this_message(messet)
  Dim sendehaeufigkeit, overhead

  'Get cycletime and calculate transmit frequency  
  if messet(6).value > 0 then    
    sendehaeufigkeit = 1000 / messet(6).value
  else
    sendehaeufigkeit = 1000 / defaultzyklus
  end if

  'retrieve overhead bits in messages. This depends on the message format - extended or standard
  if messet(3).value = 1 then 
    overhead = 77
  else
    overhead = 55
  end if

  'calculate and return busload for each message
  add_this_message = sendehaeufigkeit * (8 * messet(4).value + overhead)
end function

function valid(name)
  if (instr(name,".mdc") = len(name) - 3) or (instr(name,".dbc") = len(name) - 3) then
    valid = true
  else 
    valid = false
  end if
end function

function gettype(name)
  if instr(name,".mdc") = len(name) - 3 then
    gettype = "mdc"
  else
    gettype = "dbc"
  end if
end function


