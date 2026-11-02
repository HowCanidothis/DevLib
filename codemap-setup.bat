@echo off
rem Codemap setup: installs skills from claude-skills\ (if any), rebuilds codemap indexes
rem (the whole project if it has a local codemap.conf, otherwise DevLib and sibling submodules)
rem and packs skills as zip files for upload to a Claude account.
setlocal
cd /d "%~dp0"
set "DEVLIB=%CD%"

set "PY="
where py >nul 2>nul && set "PY=py -3"
if not defined PY (where python >nul 2>nul && set "PY=python")
if not defined PY (echo ERROR: Python 3 not found. Install it from python.org and run again. & pause & exit /b 1)

echo [1/3] Skills...
if exist "claude-skills\" (
  for /d %%S in ("claude-skills\*") do (
    if exist ".claude\skills\%%~nxS" rmdir /s /q ".claude\skills\%%~nxS"
    if not exist ".claude\skills" mkdir ".claude\skills"
    move "%%S" ".claude\skills\" >nul
    echo   installed %%~nxS
  )
  rmdir "claude-skills" 2>nul
)
for %%O in (codemap-devlib codemap-devlib-update wps-devlib-folder-access) do (
  if exist ".claude\skills\%%O" (rmdir /s /q ".claude\skills\%%O" & echo   removed old %%O)
)

echo.
echo [2/3] Building codemap indexes...
if exist "..\..\codemap.conf" (
  pushd "..\.."
  %PY% "%DEVLIB%\codemap.py" sync
  %PY% codemap.py reindex
  popd
) else (
  %PY% codemap.py reindex
  for /d %%M in (..\*) do (
    if exist "%%M\codemap.conf" if /i not "%%~fM"=="%DEVLIB%" (
      pushd "%%M"
      %PY% "%DEVLIB%\codemap.py" sync
      %PY% codemap.py reindex
      popd
    )
  )
)

echo.
echo [3/3] Packing skills...
set "DESK="
for /f "usebackq delims=" %%D in (`powershell -NoProfile -Command "[Environment]::GetFolderPath('Desktop')"`) do set "DESK=%%D"
if not defined DESK set "DESK=%USERPROFILE%\Desktop"
%PY% codemap.py zip-skills "%DESK%\Claude-skills"
echo.
echo Upload all zip files from %DESK%\Claude-skills in Claude: Settings - Skills (only needed when skills changed).
explorer "%DESK%\Claude-skills"
pause
