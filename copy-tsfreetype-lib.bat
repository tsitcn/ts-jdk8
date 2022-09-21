SET OBJS=tsfreetype-2.11.0\objs

SET DLL=Release\tsfreetype.dll
SET LIB=Release\tsfreetype.lib

copy %OBJS%\x64\%DLL%   lib-windows-32
copy %OBJS%\x64\%LIB%   lib-windows-64

copy %OBJS%\Win32\%DLL% lib-windows-32
copy %OBJS%\Win32\%LIB% lib-windows-32

SET TSO=Z:\tsoffice-3.1\office
copy %OBJS%\x64\%DLL%   %TSO%\tsjdk64\jre\bin
copy %OBJS%\Win32\%DLL% %TSO%\tsjdk32\jre\bin

PAUSE
