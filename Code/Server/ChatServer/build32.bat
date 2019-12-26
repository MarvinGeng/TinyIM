mkdir Build64Bat
cd Build64Bat
cmake .. -G "Visual Studio 15 2017 Win64"
MSBuild.exe Project.sln -property:Configuration=Debug
cd ..
pause
xcopy /y /F %cd%\include\thirdparty\mysql\lib\libeay32.dll %cd%\bin\Debug\
xcopy /y /F %cd%\include\thirdparty\mysql\lib\libmysql.dll %cd%\bin\Debug\