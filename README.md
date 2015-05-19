fastbird engine
==============
'fastbird engine' was initially started for self-teaching purpose in 2013 and now I'm trying
to finish a complete game with it. Currently the engine is martured enough to create a
simple space simulation game and I'm continually updating new codes and
funtionalities as I'm working on the game.

Compilation
---------------
You need these libraries to compile the source code. You can find the '.bat' files which helps you to set libary path environment variables easily. They are located in 'code/' folder.
* _FASTBIRD_ENGINE_DIR
  * Run _FASTBIRD_ENGINE_DIR.bat in code/ directory.
* FreeImage 3.15.4
  * Copy code/_FREEIMAGE(3.15.4)_DIR.bat file to your FreeImage directory and run. This will create $(FREEIMAGE_DIR) environment variable and the variable is used in the project setting.
* Lua 5.2.3 
  * Copy code/_LUA_DIR(5.2.3).bat file to your Lua directory and run. This will create $(LUA_DIR) environment variable.
* Zlib 1.2.8 
  * Copy code/_ZLIB(1.2.8)_HOME.bat file to your zlib directory and run. This will create $(ZLIB_HOME) environment variable.
* OpenCollada 
  * Copy code/_OpenCOLLADA_HOME.bat file to your opencolldada directory and run. This will create $(OpenCOLLADA_HOME) environment variable.
* Bullet Physics 2.82
  * copy code/_BULLET(2.82)_DIR.bat file to your Bullet directory and run. This will create $(BULLET_DIR) environment variable.
 
The project is being developed in Visual Studio 2013 Community.
 

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

* Particle HOW-TO : https://github.com/fastbird/fastbirdEngine/wiki/Particle-How-to

* UI HOW_TO : https://github.com/fastbird/fastbirdEngine/wiki/UI-How-to

* Lua Scripting HOW-TO : https://github.com/fastbird/fastbirdEngine/wiki/UI-Lua-Scripting---How-to
 
* Texture manipulation reference : https://github.com/fastbird/fastbirdEngine/wiki/Texture-manipulation-reference

* more : https://github.com/fastbird/fastbirdEngine/wiki


Development Log
------------------
You can find the development log at the site below
* https://www.youtube.com/user/jungwan82 (video logs and demonstrations)
* http://jungwan.net/w/ (text logs)


Author
------------------
Jungwan Byun (jungwan82 at naver.com)
* Homepage: http://jungwan.net/
* Blog: http://blog.naver.com/jungwan82/


Acknowledgements
------------------
I learend graphics/engine programming from these people and organization.<br>
Many thanks to...

* Angel, David, Jinho, Jordi, Junhwan, Sunggyun @ Blueside
* David @ XLGames
