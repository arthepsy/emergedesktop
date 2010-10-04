require "dl/import"

module Win32
  extend DL::Importable
  dlload "user32.dll"

  typealias "WPARAM", "UINT"
  typealias "LPARAM", "UINT"
  typealias "LPCTSTR", "CHAR *"

  extern "UINT SendMessage(HWND, UINT, WPARAM, LPARAM)"
  extern "HWND FindWindow(LPCTSTR, LPCTSTR)"
  extern "UINT RegisterWindowMessage(LPCTSTR)"
end

module Emerge
  EMERGE_DISPATCH = Win32::registerWindowMessage("EmergeDispatch")
  EMERGE_CORE = 1
  CORE_HIDE = 102
end

emergeCoreWnd = Win32::findWindow("EmergeDesktopCore", nil)
Win32::sendMessage(emergeCoreWnd, Emerge::EMERGE_DISPATCH, Emerge::EMERGE_CORE, Emerge::CORE_HIDE)
