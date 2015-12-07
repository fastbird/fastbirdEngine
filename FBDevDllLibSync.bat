mkdir DLLLibSync
xcopy Release\FBDevDllLibSync.exe DLLLibSync\ /D /E /H /Y /I
xcopy Release\FBFileSystem.dll DLLLibSync\ /D /E /H /Y /I
xcopy Release\FBFileMonitor.dll DLLLibSync\ /D /E /H /Y /I
DllLibSync\FBDevDllLibSync.exe -s ./Debug;./Release -d ../FBGame1/Code/Debug;../FBGame1/Code/Release; -f dll;lib;pdb
