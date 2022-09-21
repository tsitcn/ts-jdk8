SET ZIPFILE=tsjdk8-project
del  %ZIPFILE%.zip

"C:\Program Files\7-Zip"\7z a -tzip %ZIPFILE%.zip %ZIPFILE% -xr!.svn
