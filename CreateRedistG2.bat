@ECHO OFF
CD "%~dp0"
SET "TARGET_DIR=Gothic2-GD3D11-17.7-devX"

CALL PrepareRedist.bat "%TARGET_DIR%"

COPY Release\ddraw.dll "%TARGET_DIR%\"

