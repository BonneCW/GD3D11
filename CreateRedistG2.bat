@ECHO OFF
CD "%~dp0"
SET "TARGET_DIR=redist_g2"

CALL PrepareRedist.bat "%TARGET_DIR%"

COPY Release\ddraw.dll "%TARGET_DIR%\"

