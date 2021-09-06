@ECHO OFF
TITLE Upload files
ECHO Trying to upload files to DEV...
cd ../../
scp -r Lab2-IC_ADC_PWM em_dev:
if errorlevel 0 (
  echo success!
)
PAUSE
