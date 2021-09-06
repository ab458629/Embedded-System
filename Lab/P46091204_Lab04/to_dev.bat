@ECHO OFF
TITLE Upload files
ECHO Trying to upload files to DEV...
cd ../../
scp -r Lab4-Sensor_Serial em_dev:
if errorlevel 0 (
  echo success!
)
PAUSE
