mstsc_hole
------------

当mstsc全屏运行，且选择"应用Windows组合键"到"远程计算机"时，所有的组合键都被mstsc截获。
此时外部的SetWindowsHookEx收不到任何按键信息，无论是先启动还是后启动。

本程序hook了mstscax.dll，可以将指定的组合键"漏"出去，方便其他程序使用(主要是多桌面管理器)。

如何使用:
可以写一个bat:  mstsc_hole xxxx.rdp
也可以将rdp文件的打开方式更改为mstsc_hole
mstsc_hole.exe与ant.dll必须放在一起。

