@echo off

cd /d "%~dp0"

git init
git submodule add -b main https://github.com/libxse/commonlibsf.git lib/commonlibsf
git submodule update --init --remote --recursive

pause
