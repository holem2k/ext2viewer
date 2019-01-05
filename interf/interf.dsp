# Microsoft Developer Studio Project File - Name="interf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=interf - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "interf.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "interf.mak" CFG="interf - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "interf - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "interf - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "interf - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib ole32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "interf - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "interf - Win32 Release"
# Name "interf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\bcache.cpp
# End Source File
# Begin Source File

SOURCE=.\copy.cpp
# End Source File
# Begin Source File

SOURCE=.\copycore.cpp
# End Source File
# Begin Source File

SOURCE=.\copydlg.cpp
# End Source File
# Begin Source File

SOURCE=.\dev.cpp
# End Source File
# Begin Source File

SOURCE=.\disk.cpp
# End Source File
# Begin Source File

SOURCE=.\diskbar.cpp
# End Source File
# Begin Source File

SOURCE=.\eo_comp.cpp
# End Source File
# Begin Source File

SOURCE=.\eo_folder.cpp
# End Source File
# Begin Source File

SOURCE=.\eo_folder_core.cpp
# End Source File
# Begin Source File

SOURCE=.\exp_core.cpp
# End Source File
# Begin Source File

SOURCE=.\explorer.cpp
# End Source File
# Begin Source File

SOURCE=.\ext2_core.cpp
# End Source File
# Begin Source File

SOURCE=.\ext2_fs.cpp
# End Source File
# Begin Source File

SOURCE=.\file_propdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\filebar.cpp
# End Source File
# Begin Source File

SOURCE=.\files_propdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\finddlg.cpp
# End Source File
# Begin Source File

SOURCE=.\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\interf.cpp
# End Source File
# Begin Source File

SOURCE=.\interf.rc
# End Source File
# Begin Source File

SOURCE=.\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\mi_propdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\overdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\props.cpp
# End Source File
# Begin Source File

SOURCE=.\qsort.cpp
# End Source File
# Begin Source File

SOURCE=.\readdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\res_cache.cpp
# End Source File
# Begin Source File

SOURCE=.\showdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\splitter.cpp
# End Source File
# Begin Source File

SOURCE=.\stat.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\text.cpp
# End Source File
# Begin Source File

SOURCE=.\trace.cpp
# End Source File
# Begin Source File

SOURCE=.\view.cpp
# End Source File
# Begin Source File

SOURCE=.\warn_dlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aboutdlg.h
# End Source File
# Begin Source File

SOURCE=.\bcache.h
# End Source File
# Begin Source File

SOURCE=.\copy.h
# End Source File
# Begin Source File

SOURCE=.\copycore.h
# End Source File
# Begin Source File

SOURCE=.\copydlg.h
# End Source File
# Begin Source File

SOURCE=.\copyhelp.h
# End Source File
# Begin Source File

SOURCE=.\dev.h
# End Source File
# Begin Source File

SOURCE=.\disk.h
# End Source File
# Begin Source File

SOURCE=.\diskbar.h
# End Source File
# Begin Source File

SOURCE=.\eo_comp.h
# End Source File
# Begin Source File

SOURCE=.\eo_folder.h
# End Source File
# Begin Source File

SOURCE=.\eo_folder_core.h
# End Source File
# Begin Source File

SOURCE=.\exp_core.h
# End Source File
# Begin Source File

SOURCE=.\explorer.h
# End Source File
# Begin Source File

SOURCE=.\ext2_core.h
# End Source File
# Begin Source File

SOURCE=.\ext2_fs.h
# End Source File
# Begin Source File

SOURCE=.\file_propdlg.h
# End Source File
# Begin Source File

SOURCE=.\filebar.h
# End Source File
# Begin Source File

SOURCE=.\files_propdlg.h
# End Source File
# Begin Source File

SOURCE=.\finddlg.h
# End Source File
# Begin Source File

SOURCE=.\icons.h
# End Source File
# Begin Source File

SOURCE=.\interf.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\mi_propdlg.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\overdlg.h
# End Source File
# Begin Source File

SOURCE=.\partition.h
# End Source File
# Begin Source File

SOURCE=.\props.h
# End Source File
# Begin Source File

SOURCE=.\qsort.h
# End Source File
# Begin Source File

SOURCE=.\readdlg.h
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\res.h
# End Source File
# Begin Source File

SOURCE=.\res_cache.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\showdlg.h
# End Source File
# Begin Source File

SOURCE=.\splitter.h
# End Source File
# Begin Source File

SOURCE=.\stat.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\text.h
# End Source File
# Begin Source File

SOURCE=.\trace.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\warn_dlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\blofile.ico
# End Source File
# Begin Source File

SOURCE=.\res\blofile_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\charfile.ico
# End Source File
# Begin Source File

SOURCE=.\res\charfile_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\computer_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\ext2.ico
# End Source File
# Begin Source File

SOURCE=.\res\ext2small.ico
# End Source File
# Begin Source File

SOURCE=.\fd3_5.ico
# End Source File
# Begin Source File

SOURCE=.\res\fd3_5.ico
# End Source File
# Begin Source File

SOURCE=.\res\fd3_5_16.ico
# End Source File
# Begin Source File

SOURCE=.\fd5_25.ico
# End Source File
# Begin Source File

SOURCE=.\res\fd5_25.ico
# End Source File
# Begin Source File

SOURCE=.\res\fd5_25_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\fifofile.ico
# End Source File
# Begin Source File

SOURCE=.\res\fifofile_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\files.ico
# End Source File
# Begin Source File

SOURCE=.\folder.ico
# End Source File
# Begin Source File

SOURCE=.\res\folder.ico
# End Source File
# Begin Source File

SOURCE=.\res\folder_16.ico
# End Source File
# Begin Source File

SOURCE=.\hdd.ico
# End Source File
# Begin Source File

SOURCE=.\res\hdd.ico
# End Source File
# Begin Source File

SOURCE=.\res\hdd_16.ico
# End Source File
# Begin Source File

SOURCE=.\interf.ico
# End Source File
# Begin Source File

SOURCE=.\res\interf.ico
# End Source File
# Begin Source File

SOURCE=.\res\lnkfile.ico
# End Source File
# Begin Source File

SOURCE=.\res\lnkfile_16.ico
# End Source File
# Begin Source File

SOURCE=.\ofolder.ico
# End Source File
# Begin Source File

SOURCE=.\res\ofolder.ico
# End Source File
# Begin Source File

SOURCE=.\res\ofolder_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\regfile.ico
# End Source File
# Begin Source File

SOURCE=.\res\regfile_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\small.ico
# End Source File
# Begin Source File

SOURCE=.\small.ico
# End Source File
# Begin Source File

SOURCE=.\res\sockfile.ico
# End Source File
# Begin Source File

SOURCE=.\res\sockfile_16.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
