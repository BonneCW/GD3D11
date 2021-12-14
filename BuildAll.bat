@ECHO OFF
CD %~dp0

SET "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\amd64\MSBuild.exe"

REM RMDIR /s /q D3D11Engine\Release_AVX\
REM RMDIR /s /q D3D11Engine\Release\
REM RMDIR /s /q D3D11Engine\Release_G1_AVX\
REM RMDIR /s /q D3D11Engine\Release_G1\

"%MSBUILD%" Direct3D7Wrapper.sln /p:Configuration=Release_AVX
"%MSBUILD%" Direct3D7Wrapper.sln /p:Configuration=Release_G1_AVX

"%MSBUILD%" Direct3D7Wrapper.sln /p:Configuration=Release
"%MSBUILD%" Direct3D7Wrapper.sln /p:Configuration=Release_G1

"%MSBUILD%" Direct3D7Wrapper.sln /p:Configuration=Spacer_NET
