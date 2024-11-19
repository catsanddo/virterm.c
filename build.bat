@echo off

if NOT EXIST bin mkdir bin

pushd bin
cl ../src/demo.c /Fe:demo.exe /I..\include /Zi
popd
