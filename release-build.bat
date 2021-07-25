cls

rmdir /s /q build
rmdir /s /q x64
del bin\ogl-demo-template.exe
del bin\ogl-demo-template.ilk
del bin\ogl-demo-template.pdb
del bin\sandbox.log

mkdir build

rc.exe res\main.rc

cl.exe /c /EHsc /O2 /I"inc" /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /Fo"build\\" src\*.cpp 

link.exe /OUT:"bin\\ogl-demo-template.exe" /MACHINE:X64 /SUBSYSTEM:WINDOWS /LIBPATH:"lib" build\main.obj build\ColorShader.obj build\helper.obj build\logger.obj build\material.obj build\ParticleShader.obj build\ParticleSystem.obj build\PBRShader.obj build\primitives.obj build\scene-square.obj build\scene-triangle.obj build\scene.obj build\shader.obj res\main.res opengl32.lib user32.lib kernel32.lib gdi32.lib glew32.lib

