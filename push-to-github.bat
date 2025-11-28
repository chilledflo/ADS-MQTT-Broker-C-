@echo off
echo.
echo ============================================
echo   Pushing to GitHub...
echo ============================================
echo.

cd C:\ADS-Realtime-CPP

echo Checking git status...
git status

echo.
echo Pushing to origin/main...
git push -u origin main

echo.
if %ERRORLEVEL% EQU 0 (
    echo ✅ SUCCESS! Repository uploaded to GitHub
    echo    https://github.com/chilledflo/ADS-Realtime-CPP
) else (
    echo ❌ ERROR: Push failed
    echo    Make sure you created the repository on GitHub first
)
echo.
pause
