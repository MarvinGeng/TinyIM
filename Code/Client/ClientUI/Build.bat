mkdir VS2017
cd VS2017
cmake ..
MSBuild.exe TinyIM.sln
cd ..
copy /y %cd%\UI_RES\ %cd%\Bin\Debug\
XCOPY /y /D /S %cd%\UI_RES\Face %cd%\Bin\Debug\Face\
XCOPY /y /D /S %cd%\UI_RES\Image %cd%\Bin\Debug\Image\
XCOPY /y /D /S %cd%\UI_RES\config %cd%\Bin\Debug\config\
XCOPY /y /D /S %cd%\UI_RES\Skins %cd%\Bin\Debug\Skins\
XCOPY /y /D /S %cd%\UI_RES\Sound %cd%\Bin\Debug\Sound\
