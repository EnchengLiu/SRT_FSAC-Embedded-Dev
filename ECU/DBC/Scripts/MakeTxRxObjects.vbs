''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' This VB-Script insert TxMessages, TxSignals and RxSignals into a CANdb++
' database version 1.1.
' Databases of version 1.0 only have relations of mapped signals and only
' one transmitting node for messages.
' Databases of CANdb++ version 1.1 have direct signal-node relations (NodeTxSigs
' and NodeRxSigs) and a message can have more than one sender (NodeTxMessages)
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
' Copyright (c) 2001 Vector Informatik GmbH.  All rights reserved.
'
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

' -------
' Globals
' -------

Dim wshShell, args, dbName, cnn

Set wshShell = WScript.CreateObject( "WScript.Shell")


' ----------------------------
' Open database
' ----------------------------

' Create an ADO connection object
Set cnn = CreateObject("ADODB.Connection")

' Define a connection by referencing the Jet driver.
Set args = WScript.Arguments
dbname = ""
if args.Count > 0 then
  dbName = WScript.Arguments(0)
end if
cnn.ConnectionString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" & dbName 

' Open connection to data source
on error resume next
cnn.Open
if err.description <> "" then
  dbName = InputBox("Please insert full path of a valid CANdb++ file", "CANdb++", "*.mdc")    
  cnn.ConnectionString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" & dbName 
  on error resume next
  cnn.open
  if err.description <> "" then
    if dbname <> "" then
      call msgbox(dbname & " is not a valid CANdb++ file",0,"CANdb++")
    end if
    wscript.quit 1
  end if
end if
on error goto 0


' -----------------------
' Initialize database IDs
' -----------------------
Dim rsMessage, messageID, senderId, rsNodeTxMessage
set rsMessage = cnn.Execute("SELECT * FROM Cdb_Message")

do while not rsMessage.EOF
  messageId = rsMessage("DB_ID")
  senderId = rsMessage("Sender")
  if not IsNull(senderId) then
    Set rsNodeTxMessage  = cnn.Execute("SELECT * FROM Cdb_Node_TxMessage WHERE Node = " & senderId & " AND Message = " & messageId)
    if rsNodeTxMessage.EOF then
      ' transmitting node message relation not defined, insert new relation
      cnn.Execute("INSERT INTO Cdb_Node_TxMessage VALUES (" & senderId & ", " & messageId & ")")
    end if
  end if

  rsMessage.MoveNext
loop

Dim rsNode, rsMappedRxSigs, rsMappedTxSigs, rsRxSigs, rsTxSigs, nodeId, signalId
set rsNode = cnn.Execute("SELECT * FROM Cdb_Node")

do while not rsNode.EOF
  nodeId = rsNode("DB_ID")
  set rsMappedRxSigs = cnn.Execute("SELECT * FROM Cdb_Node_RxSignal WHERE Node = " & nodeID)

  do while not rsMappedRxSigs.EOF
    signalId = rsMappedRxSigs("Signal")
    set rsRxSigs = cnn.Execute("SELECT * FROM Cdb_Node_RxSig WHERE Node = " & nodeID & " AND Signal = " & signalID)
    
    if rsRxSigs.EOF then
       ' rx node signal relation not defined, insert new relation
       cnn.Execute("INSERT INTO Cdb_Node_RxSig VALUES (" & nodeId & ", " & signalId &")")
    end if

    rsMappedRxSigs.MoveNext
  loop

  set rsNodeTxMessage = cnn.Execute("SELECT * FROM Cdb_Node_TxMessage WHERE Node = " & nodeID)

  do while not rsNodeTxMessage.EOF
    messageId = rsNodeTxMessage("Message")
    set rsMappedTxSigs = cnn.Execute("SELECT * FROM Cdb_Message_Signal WHERE Message = " & messageID)

    do while not rsMappedTxSigs.EOF
      signalId = rsMappedTxSigs("Signal")
      set rsTxSigs = cnn.Execute("SELECT * FROM Cdb_Node_TxSig WHERE Node = " & nodeID & " AND Signal = " & signalID)
    
      if rsTxSigs.EOF then
         ' tx node signal relation not defined, insert new relation
         cnn.Execute("INSERT INTO Cdb_Node_TxSig VALUES (" & nodeId & ", " & signalId &")")
      end if

      rsMappedTxSigs.MoveNext
    loop

    rsNodeTxMessage.MoveNext
  loop

  rsNode.MoveNext
loop

' ------------------------------------
' Close the connection to the database
' ------------------------------------
cnn.Close

MsgBox("Node Message relation inserted successfully!")

wscript.quit 2

' -------------------
' End of main program
' -------------------
