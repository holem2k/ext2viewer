При замене диска
Refresh:
1) Сворачивание дерева
WNDPROC -> dispatch_explorer(..., OP_REFRESH, ...) -> eo_folder_refresh (PostMessage(..., WM_REFRESH, ...));
2) Создание нового дерева
WNDPROC -> control_explorer(..., E_REFRESH_REMOVABLE, ...) -> refresh_removable() (TV_SelectItem)
3) Выделение вершины дерева
WNDPROC -> dispatch_explorer(..., OP_OPEN, ...) -> ...

При ошибке монтирования:
1) При выборе  любого каталога кроме корневого - Retry/Cancel
2) При выборе корневого каталога - Retry/Cancel, Если Cancel -  то  refresh


При ошибке копирования:
1) если не читается ext2 раздел - Повторить/Отмена(всей операции копирования)
2) если не пишется FAT/NTFS - Повторить/Отмена(всей операции копирования)
TODO  Повторить/Пропустить(копирование файла)/Отмена(всей операции копирования)
