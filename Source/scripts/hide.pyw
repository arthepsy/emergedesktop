from win32gui import FindWindow
from win32gui import SendMessage
from win32gui import RegisterWindowMessage

EMERGE_DISPATCH = RegisterWindowMessage("EmergeDispatch");

EMERGE_CORE = 1
CORE_HIDE = 102

emergeCoreWnd = FindWindow("EmergeDesktopCore", None);
SendMessage(emergeCoreWnd, EMERGE_DISPATCH, EMERGE_CORE, CORE_HIDE);
