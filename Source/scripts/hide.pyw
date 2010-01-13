from win32gui import FindWindow
from win32gui import SendMessage

WM_USER = 0x0400
EMERGE_DISPATCH = WM_USER +4

EMERGE_CORE = 1
CORE_HIDE = 102

emergeCoreWnd = FindWindow("emergeCoreClass", None);
SendMessage(emergeCoreWnd, EMERGE_DISPATCH, EMERGE_CORE, CORE_HIDE);
