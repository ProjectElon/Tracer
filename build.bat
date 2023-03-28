@echo off
set Defines=-DTRACER_DEBUG=1 -DTRACER_INTERNAL=1 -DTRACER_ASSERTIONS=1 -D_CRT_SECURE_NO_WARNINGS
set Includes=-I../vendor -I../source/vendor
set LibIncludes=-libpath:../vendor/libs
set DebugFlags=-FC -Zi
set CompilerFlags=-nologo -MT -Gm- -GR- -EHa- -O2 -fp:fast -W4 -wd4201 -wd4100 -wd4189
set ExecutableName=tracer
set CodePath=../source/
set Win32Libs=kernel32.lib user32.lib gdi32.lib opengl32.lib glfw3dll.lib
set LinkFlags=-subsystem:console -opt:ref
pushd build
cl %Defines% %DebugFlags% %CompilerFlags% %Includes% -Fe%ExecutableName% %CodePath%tracer_main.cpp %Win32Libs% /link %LinkFlags% %LibIncludes%
popd