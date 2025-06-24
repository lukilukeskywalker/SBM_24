@echo off
setlocal enabledelayedexpansion

REM Check arguments
if "%~2"=="" (
    echo Usage: rename.bat [source_folder] [new_name]
    exit /b 1
)

set "SRC_FOLDER=%~1"
set "NEW_NAME=%~2"

REM Get the name of the source folder only
for %%F in ("%SRC_FOLDER%") do set "SRC_NAME=%%~nxF"

REM Copy folder
xcopy "%SRC_FOLDER%" "%NEW_NAME%" /E /I /H /Y

REM Rename .uvprojx and .uvoptx files
pushd "%NEW_NAME%"
for %%f in (*.uvprojx *.uvoptx *.uvguix *.axf) do (
    set "OLD_FILE=%%f"
    set "EXT=%%~xf"
    ren "!OLD_FILE!" "%NEW_NAME%!EXT!"
)

REM Replace contents inside the project files
for %%f in (%NEW_NAME%.uvprojx %NEW_NAME%.uvoptx %NEW_NAME%.uvguix %NEW_NAME%.axf) do (
    powershell -Command "(Get-Content -Raw '%%f') -replace '%SRC_NAME%', '%NEW_NAME%' | Set-Content '%%f'"
)

popd
echo Project renamed successfully to %NEW_NAME%
