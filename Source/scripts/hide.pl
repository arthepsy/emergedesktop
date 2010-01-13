use Win32::GUI;

use constant WM_USER => 0x0400;

my $EMERGE_DISPATCH = WM_USER + 4;
my $EMERGE_CORE = 1;
my $CORE_HIDE = 102;

my $emergeCoreWnd = Win32::GUI::FindWindow("emergeCoreClass", undef);
Win32::GUI::SendMessage($emergeCoreWnd, $EMERGE_DISPATCH, $EMERGE_CORE, $CORE_HIDE);
