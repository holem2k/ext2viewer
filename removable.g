�� ������ ��᪠
Refresh:
1) ����稢���� ��ॢ�
WNDPROC -> dispatch_explorer(..., OP_REFRESH, ...) -> eo_folder_refresh (PostMessage(..., WM_REFRESH, ...));
2) �������� ������ ��ॢ�
WNDPROC -> control_explorer(..., E_REFRESH_REMOVABLE, ...) -> refresh_removable() (TV_SelectItem)
3) �뤥����� ���設� ��ॢ�
WNDPROC -> dispatch_explorer(..., OP_OPEN, ...) -> ...

�� �訡�� ����஢����:
1) �� �롮�  ��� ��⠫��� �஬� ��୥���� - Retry/Cancel
2) �� �롮� ��୥���� ��⠫��� - Retry/Cancel, �᫨ Cancel -  �  refresh


�� �訡�� ����஢����:
1) �᫨ �� �⠥��� ext2 ࠧ��� - �������/�⬥��(�ᥩ ����樨 ����஢����)
2) �᫨ �� ������ FAT/NTFS - �������/�⬥��(�ᥩ ����樨 ����஢����)
TODO  �������/�ய�����(����஢���� 䠩��)/�⬥��(�ᥩ ����樨 ����஢����)
