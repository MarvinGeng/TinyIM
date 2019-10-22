mkdir Build32Bat
cd Build32Bat
cmake ..
MSBuild.exe Project.sln -property:Configuration=Debug
pause
