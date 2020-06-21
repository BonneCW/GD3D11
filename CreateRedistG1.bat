@ECHO OFF
CD "%~dp0"
SET "TARGET_DIR=redist_g1"

CALL PrepareRedist.bat "%TARGET_DIR%"

COPY Release_G1\ddraw.dll "%TARGET_DIR%\"

