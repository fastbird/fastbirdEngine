Fastbird Engine
==============
Open source game engine.

Compilation
---------------
You need these libraries to compile the source code. You can find the helper .bat files setting environment variables for these paths in the code/ folder.
* FreeImage 3.15.4 : Copy code/_FREEIMAGE(3.15.4)_DIR.bat file to your FreeImage directory and run. This will create $(FREEIMAGE_DIR) environment variable and the variable is used in the project setting.
* Lua 5.2.3 : Copy code/_LUA_DIR(5.2.3).bat file to your Lua directory and run. This will create $(LUA_DIR) environment variable.
* Zlib 1.2.8 : Copy code/_ZLIB(1.2.8)_HOME.bat file to your zlib directory and run. This will create $(ZLIB_HOME) environment variable.
* OpenCollada : Copy code/_OpenCOLLADA_HOME.bat file to your opencolldada directory and run. This will create $(OpenCOLLADA_HOME) environment variable.
 
The project is developing in Visual Studio 2013 express edition.
 

How-tos
-------------

* Basic HOW-TOs: https://github.com/fastbird/fastbirdEngine/wiki/2014-July-How-to
  * initialize the engine
  * create sky
  * load a collada model and material
  * use voxelizer
  * use render to texture system
  * profile the performance
  * do parallel computing

* Material HOW-TO : https://github.com/fastbird/fastbirdEngine/wiki/Material-How-to

* Particle HOW_TO : https://github.com/fastbird/fastbirdEngine/wiki/Particle-How-to

Development Log
------------------
You can find the development log at the site below
* http://jungwan.net/w/


Author
------------------
Jungwan Byun (jungwan82 at naver.com)
* Homepage: http://jungwan.net/
* Blog: http://blog.naver.com/jungwan82/


Special Thanks to
------------------
I learend graphics/engine programming from these people.
Angel, David, Jinho, Jordi, Junhwan, Sunggyun @ Blueside
David @ XLGames
Thank you guys. :)
