# Microsoft Developer Studio Project File - Name="DCPlusPlus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DCPlusPlus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DCPlusPlus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DCPlusPlus.mak" CFG="DCPlusPlus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DCPlusPlus - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DCPlusPlus - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DCPlusPlus - Win32 Release"

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
# ADD CPP /nologo /G6 /Gr /MT /W4 /GX /Zi /Og /Oi /Os /Oy /Ob2 /Gy /D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "_REENTRANT" /FAs /Yu"stdafx.h" /Gs256 /FD /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib comctl32.lib Ws2_32.lib wininet.lib mswsock.lib shlwapi.lib /nologo /version:0.15 /subsystem:windows /map /machine:I386
# SUBTRACT LINK32 /profile /debug

!ELSEIF  "$(CFG)" == "DCPlusPlus - Win32 Debug"

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
# ADD CPP /nologo /G6 /Gr /MTd /W4 /Gm /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "STRICT" /D "_REENTRANT" /Yu"stdafx.h" /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib comctl32.lib Ws2_32.lib wininet.lib mswsock.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /profile /map

!ENDIF 

# Begin Target

# Name "DCPlusPlus - Win32 Release"
# Name "DCPlusPlus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\client\AdvancedPage.cpp
# End Source File
# Begin Source File

SOURCE=.\client\AppearancePage.cpp
# End Source File
# Begin Source File

SOURCE=.\client\BitInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\client\BitOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\client\BufferedSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ClientManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ConnectionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\client\CryptoManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DCPlusPlus.cpp
# End Source File
# Begin Source File

SOURCE=.\DCPlusPlus.rc
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListing.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListingFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DownloadManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\DownloadPage.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ExListViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\client\FavHubProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\client\FavoritesFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\File.cpp
# End Source File
# Begin Source File

SOURCE=.\client\FlatTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\client\GeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=.\client\HttpConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\client\HubFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\client\HubManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\NotepadFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Pointer.cpp
# End Source File
# Begin Source File

SOURCE=.\client\PrivateFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\client\PropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\client\PropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\client\PublicHubsFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\QueueFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\client\QueueManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\SearchFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\client\SearchManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ServerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\client\SettingsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\ShareManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\SimpleXML.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Socket.cpp
# End Source File
# Begin Source File

SOURCE=.\client\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\client\StringDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\client\StringTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\client\TimerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\UploadManager.cpp
# End Source File
# Begin Source File

SOURCE=.\client\UploadPage.cpp
# End Source File
# Begin Source File

SOURCE=.\client\User.cpp
# End Source File
# Begin Source File

SOURCE=.\client\UserConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\client\Util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\client\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\client\AdvancedPage.h
# End Source File
# Begin Source File

SOURCE=.\client\AppearancePage.h
# End Source File
# Begin Source File

SOURCE=.\client\AtlCmdBar2.h
# End Source File
# Begin Source File

SOURCE=.\client\BitInputStream.h
# End Source File
# Begin Source File

SOURCE=.\client\BitOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\client\BufferedSocket.h
# End Source File
# Begin Source File

SOURCE=.\client\Client.h
# End Source File
# Begin Source File

SOURCE=.\client\ClientManager.h
# End Source File
# Begin Source File

SOURCE=.\client\config.h
# End Source File
# Begin Source File

SOURCE=.\client\ConnectionManager.h
# End Source File
# Begin Source File

SOURCE=.\client\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\client\CryptoManager.h
# End Source File
# Begin Source File

SOURCE=.\client\DCPlusPlus.h
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListing.h
# End Source File
# Begin Source File

SOURCE=.\client\DirectoryListingFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\DownloadManager.h
# End Source File
# Begin Source File

SOURCE=.\client\DownloadPage.h
# End Source File
# Begin Source File

SOURCE=.\client\Exception.h
# End Source File
# Begin Source File

SOURCE=.\client\ExListViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\client\FavHubProperties.h
# End Source File
# Begin Source File

SOURCE=.\client\FavoritesFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\File.h
# End Source File
# Begin Source File

SOURCE=.\client\FlatTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\client\GeneralPage.h
# End Source File
# Begin Source File

SOURCE=.\client\HttpConnection.h
# End Source File
# Begin Source File

SOURCE=.\client\HubFrame.h
# End Source File
# Begin Source File

SOURCE=.\client\HubManager.h
# End Source File
# Begin Source File

SOURCE=.\client\LineDlg.h
# End Source File
# Begin Source File

SOURCE=.\client\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\NotepadFrame.h
# End Source File
# Begin Source File

SOURCE=.\client\Pointer.h
# End Source File
# Begin Source File

SOURCE=.\client\PrivateFrame.h
# End Source File
# Begin Source File

SOURCE=.\client\PropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\client\PropPage.h
# End Source File
# Begin Source File

SOURCE=.\client\PublicHubsFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\QueueFrame.h
# End Source File
# Begin Source File

SOURCE=.\client\QueueManager.h
# End Source File
# Begin Source File

SOURCE=.\client\resource.h
# End Source File
# Begin Source File

SOURCE=.\client\ResourceManager.h
# End Source File
# Begin Source File

SOURCE=.\client\SearchFrm.h
# End Source File
# Begin Source File

SOURCE=.\client\SearchManager.h
# End Source File
# Begin Source File

SOURCE=.\client\Semaphore.h
# End Source File
# Begin Source File

SOURCE=.\client\ServerSocket.h
# End Source File
# Begin Source File

SOURCE=.\client\SettingsManager.h
# End Source File
# Begin Source File

SOURCE=.\client\ShareManager.h
# End Source File
# Begin Source File

SOURCE=.\client\SimpleXML.h
# End Source File
# Begin Source File

SOURCE=.\client\Socket.h
# End Source File
# Begin Source File

SOURCE=.\client\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\client\StringDefs.h
# End Source File
# Begin Source File

SOURCE=.\client\StringTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\client\TimerManager.h
# End Source File
# Begin Source File

SOURCE=.\client\UploadManager.h
# End Source File
# Begin Source File

SOURCE=.\client\UploadPage.h
# End Source File
# Begin Source File

SOURCE=.\client\User.h
# End Source File
# Begin Source File

SOURCE=.\client\UserConnection.h
# End Source File
# Begin Source File

SOURCE=.\client\Util.h
# End Source File
# Begin Source File

SOURCE=.\client\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\arrows.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\client\res\DCPlusPlus.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlus.ico
# End Source File
# Begin Source File

SOURCE=.\client\res\DCPlusPlusdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCPlusPlusdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Directory.ico
# End Source File
# Begin Source File

SOURCE=.\res\Favorites.ico
# End Source File
# Begin Source File

SOURCE=.\res\folders.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hub.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_mdid.ico
# End Source File
# Begin Source File

SOURCE=.\res\mdichild.bmp
# End Source File
# Begin Source File

SOURCE=.\res\notepad.ico
# End Source File
# Begin Source File

SOURCE=.\res\PublicHubs.ico
# End Source File
# Begin Source File

SOURCE=.\res\Search.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar20.bmp
# End Source File
# Begin Source File

SOURCE=.\res\User.ico
# End Source File
# Begin Source File

SOURCE=.\res\users.bmp
# End Source File
# End Group
# End Target
# End Project
