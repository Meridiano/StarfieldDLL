#NoTrayIcon
#include <WinAPIProc.au3>
#include <GUIConstantsEx.au3>
#include <WindowsConstants.au3>

; setup gui
Local $iStyle = $WS_BORDER + $WS_POPUP
Local $iStyleEx = $WS_EX_TOPMOST + $WS_EX_LAYERED
Local $hGUI = GUICreate(@ScriptName, @DesktopWidth, @DesktopHeight, 0, 0, $iStyle, $iStyleEx)

; set transparent
GUISetBkColor(0, $hGUI)
WinSetTrans($hGUI, "", 1)

; show gui
GUISetState(@SW_SHOW, $hGUI)

; game search
Func GameRunning()
	Local $aList = ProcessList("Starfield.exe")
	For $iIndex = 1 To $aList[0][0]
		Local $iPID = $aList[$iIndex][1]
		Local $sGamePath = _WinAPI_GetProcessFileName($iPID)
		Local $sSelfPath = StringRegExpReplace($sGamePath, "^(.+)\\([^\\]+)$", "$1\\Data\\SFSE\\Plugins\\" & @ScriptName)
		If $sSelfPath = @ScriptFullPath Then Return True
	Next
	Return False
EndFunc

; min-max
Func ValueMinMax($iVal, $iMin, $iMax)
	If ($iVal < $iMin) Then Return $iMin
	If ($iVal > $iMax) Then Return $iMax
	Return $iVal
EndFunc

; set timeout
Local $sConfig = StringTrimRight(@ScriptFullPath, 3) & "ini"
Local $bTimeOut = (Number(IniRead($sConfig, "General", "bTimeOut", "0")) <> 0)
Local $iTimeOutMS = ValueMinMax(Number(IniRead($sConfig, "General", "iTimeOutMS", "120000")), 5000, 600000)
Local $hTimeOut = TimerInit()

; run loop
While True
	Sleep(100)
	WinActivate($hGUI, "")
	If Not GameRunning() Then Exit
	If $bTimeOut And (TimerDiff($hTimeOut) > $iTimeOutMS) Then Exit
WEnd
