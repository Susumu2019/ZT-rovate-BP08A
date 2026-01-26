@echo off
REM 仮想環境のpython.exeを絶対パスで実行
set SCRIPT_PATH=%~dp0udpcontrol.py
set VENV_PYTHON=%~dp0..\..\.venv\Scripts\python.exe
"%VENV_PYTHON%" "%SCRIPT_PATH%"
