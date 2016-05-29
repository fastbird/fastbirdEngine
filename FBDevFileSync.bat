xcopy Release\FBDevFileSync.exe FileSync\ /D /E /H /Y /I
xcopy Release\FBFileSystem.dll FileSync\ /D /E /H /Y /I
xcopy Release\FBFileMonitor.dll FileSync\ /D /E /H /Y /I
xcopy Release\FBThread.dll FileSync\ /D /E /H /Y /I
FileSync\FBDevFileSync.exe -s ./Debug;./Release -d ../FBGame1/Code/Debug;../FBGame1/Code/Release;../Globe/Debug;../Globe/Release; -f dll;lib;pdb
