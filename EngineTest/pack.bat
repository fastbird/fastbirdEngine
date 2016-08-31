cd Data

..\..\Release\FBDevColladaToFBMesh.exe -e ../mesh_exclude.txt

cd ../

%FASTBIRD_ENGINE_DIR%\Release\FBDevDataPacker.exe b -v -p fastbird -e pack_exclude.txt -r 0 Data

PAUSE
