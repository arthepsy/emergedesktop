#require "Win32API"

#WM_USER=0x0400
#EMERGE_DISPATCH=WM_USER + 4
#EMERGE_CORE=1
#CORE_HIDE=102

#findWindow = Win32API.new("user32", 
#                          "FindWindow", 
#                          ['P','P'], 'L')
#sendMessage = Win32API.new("user32", "SendMessage",
#                            ['L','L','L','P'], 'L')

#emergeCoreWnd = findWindow.call("emergeCoreClass", 0)
#sendMessage.call(emergeCoreWnd, EMERGE_DISPATCH, EMERGE_CORE, CORE_HIDE)

require "dl/import"

module Win32
	extend DL::Importable
	dlload "user32.dll"

	WM_USER = 0x0400

	typealias "WPARAM", "UINT"
	typealias "LPARAM", "UINT"
	typealias "LPCTSTR", "CHAR *"
	
	extern "UINT SendMessage(HWND, UINT, WPARAM, LPARAM)"
	extern "HWND FindWindow(LPCTSTR, LPCTSTR)"
end

module Emerge
	EMERGE_DISPATCH = Win32::WM_USER + 4
	EMERGE_CORE = 1
	CORE_HIDE = 102
end

emergeCoreWnd = Win32::findWindow("emergeCoreClass", nil)
Win32::sendMessage(emergeCoreWnd, Emerge::EMERGE_DISPATCH, Emerge::EMERGE_CORE, Emerge::CORE_HIDE)
