@echo off
echo Checking out project and filters file
p4 edit evelocalization.vcxproj
p4 edit evelocalization.filters
echo Regenerating
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\..\carbon\tools\ProjectFileGenerator\ProjectFileGenerator.py -i evelocalization.ccpproj
pause