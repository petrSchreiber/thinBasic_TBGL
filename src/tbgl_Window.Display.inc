type tbgl_tDisplayInfo
  index    as long

  widthPx    as long
  heightPx   as long

  leftPx     as long
  rightPx    as long

  topPx      as long
  bottomPx   as long
end type

' --

function DisplayProc_Counter(byVal hMonitor as dword, byVal hDc as dword, byRef r as RECT, byVal userData as dword) as long
  dim counter(0) as dword at userData
  incr counter(0)

  function = 1
end function

' --

type tbgl_tDisplayWantedInfo
  currentIndex        as long
  wantedDisplayIndex  as long

  info    as tbgl_tDisplayInfo
end type

function DisplayProc_GetInfoForWantedDisplay(byVal hMonitor as dword, byVal hDc as dword, byRef r as RECT, byVal userData as dword) as long
  dim wanted(0) as tbgl_tDisplayWantedInfo at userData
  incr wanted(0).currentIndex

  ' -- Works ok even if Index is not defined
  if wanted(0).currentIndex = wanted(0).wantedDisplayIndex then
    wanted(0).info.leftPx   = r.nLeft
    wanted(0).info.rightPx  = r.nRight
    wanted(0).info.topPx    = r.nTop
    wanted(0).info.bottomPx = r.nBottom

    wanted(0).info.widthPx  = r.nRight - r.nLeft
    wanted(0).info.heightPx = r.nBottom - r.nTop

    function = 0
  else
    function = 1
  end if

end function

' --

type tbgl_tDisplayByPosition
  currentIndex        as long
  foundDisplayIndex   as long

  checkedWindowTop    as long
  checkedWindowLeft   as long
end type

function DisplayProc_FindDisplay(byVal hMonitor as dword, byVal hDc as dword, byRef r as RECT, byVal userData as dword) as long
  dim windowSearchData(0) as tbgl_tDisplayByPosition at userData
  incr windowSearchData(0).currentIndex

  if r.nRight > windowSearchData(0).checkedWindowLeft then
    windowSearchData(0).foundDisplayIndex = windowSearchData(0).currentIndex

    function = 0
  else
    function = 1
  end if

end function

function Exec_TBGL_DisplayGetCount() as long

  ' -- Parsing input
  if thinBasic_CheckOpenParens_Optional() then thinBasic_CheckCloseParens_Mandatory()

  ' -- Actual code
  dim displayCount as dword
  if EnumDisplayMonitors(0, byVal 0, codePtr(DisplayProc_Counter), varptr(displayCount)) <> 0 then
    function = displayCount
  else
    function = 1
  end if

end function

function internal_displayGetInfo(byVal displayNo as long, byRef info as tbgl_tDisplayInfo) as long
  dim wanted as tbgl_tDisplayWantedInfo
  wanted.wantedDisplayIndex = displayNo

  EnumDisplayMonitors(0, byVal 0, codePtr(DisplayProc_GetInfoForWantedDisplay), varptr(wanted))

  type set info = wanted.info
end function

function Exec_TBGL_DisplayGetInfo() as long

  ' -- Parsing input
  if thinBasic_CheckOpenParens_Mandatory = %FALSE or thinBasic_ErrorFree = %FALSE then exit function
  dim eDisplayNumber as ext
  thinBasic_ParseNumber eDisplayNumber

  if thinBasic_CheckComma = %FALSE or thinBasic_ErrorFree = %FALSE then exit function
  local lVariablePtr1, lVariableAbsPos1 as long
  thinBasic_VariableParse( lVariablePtr1, lVariableAbsPos1 )

  if thinBasic_CheckComma = %FALSE or thinBasic_ErrorFree = %FALSE then exit function
  local lVariablePtr2, lVariableAbsPos2 as long
  thinBasic_VariableParse( lVariablePtr2, lVariableAbsPos2 )

  if thinBasic_CheckComma_Optional and thinBasic_ErrorFree then
    local lVariablePtr3, lVariableAbsPos3 as long
    thinBasic_VariableParse( lVariablePtr3, lVariableAbsPos3 )
  end if

  if thinBasic_ErrorFree = %FALSE then exit function
  thinBasic_CheckCloseParens_Mandatory

  ' -- Actual code
  dim info as tbgl_tDisplayInfo
  internal_displayGetInfo(eDisplayNumber, info)

  local eResult1, eResult2, eResult3 as ext
  eResult1 = info.widthPx
  eResult2 = info.heightPx

  ' -- Promoting the values back to passed variables
  thinBasic_ChangeVariableNumberDirect( lVariablePtr1, lVariableAbsPos1, eResult1 )
  thinBasic_ChangeVariableNumberDirect( lVariablePtr2, lVariableAbsPos2, eResult2 )

  if lVariablePtr3 then
    dim dHdc as dword
    dHdc = GetDC( %HWND_DESKTOP )
    eResult3 = GetDeviceCaps( dHDc, %BITSPIXEL )
    ReleaseDC( %HWND_DESKTOP, dHdc )

    thinBasic_ChangeVariableNumberDirect( lVariablePtr3, lVariableAbsPos3, eResult3 )
  end if

end function

function Exec_TBGL_SendWindowToDisplay() as long

  dim wHandle as ext
  dim currentDisplay as ext
  dim eDisplay as ext
  dim newX as ext
  dim newY as ext
  dim r as RECT

  ' -- Parsing input
  if thinBasic_CheckOpenParens_Mandatory = %FALSE then exit function
  thinBasic_ParseNumber(wHandle)

  currentDisplay = internal_GetWindowDisplay(wHandle)
  dim info as tbgl_tDisplayInfo
  internal_displayGetInfo(currentDisplay, info)

  GetWindowRect(wHandle, r)

  if thinBasic_CheckComma_Mandatory = %FALSE then exit function
  thinBasic_ParseNumber(eDisplay)

  if thinBasic_CheckComma_Optional then
    thinBasic_ParseNumber(newX)
  else
    newX = r.nLeft - info.leftPx
  end if

  if thinBasic_CheckComma_Optional then
    thinBasic_ParseNumber(NewY)
  else
    newY = r.nTop - info.topPx
  end if

  if thinBasic_CheckCloseParens_Mandatory = %FALSE then exit function

  ' -- Actual code
  dim newDisplayInfo as tbgl_tDisplayInfo
  internal_displayGetInfo(eDisplay, newDisplayInfo)

  newX += newDisplayInfo.leftPx
  newY += newDisplayInfo.topPx

  if newX > newDisplayInfo.rightPx then
    newX = newDisplayInfo.leftPx
  end if

  if newY > newDisplayInfo.bottomPx then
    newY = newDisplayInfo.topPx
  end if

  GetWindowRect(wHandle, r)
  MoveWindow(wHandle, newX, newY, r.nRight-r.nLeft, r.nBottom-r.nTop, %TRUE)

end function

' --

function internal_getWindowDisplay(byVal wHandle as dword) as long
  dim wr as RECT
  dim cr as RECT

  GetWindowRect(wHandle, wr)
  GetClientRect(wHandle, cr)

  dim windowSearchData as tbgl_tDisplayByPosition
  windowSearchData.checkedWindowLeft = wr.nLeft - (((wr.nLeft-wr.nRight)-(cr.nLeft-cr.nRight))/2)
  windowSearchData.checkedWindowTop = wr.nTop
  EnumDisplayMonitors(0, byVal 0, codePtr(DisplayProc_FindDisplay), varptr(windowSearchData))

  function = windowSearchData.foundDisplayIndex

end function

function Exec_TBGL_GetWindowDisplay() as long

  dim wHandle as ext

  ' -- Parsing input
  if thinBasic_CheckOpenParens_Mandatory = %FALSE then exit function
  thinBasic_ParseNumber(wHandle)
  if thinBasic_CheckCloseParens_Mandatory = %FALSE then exit function

  ' -- Actual code
  function = internal_getWindowDisplay(wHandle)

end function
