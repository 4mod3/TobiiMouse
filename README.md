# TobiiMouse [![HitCount](http://hits.dwyl.io/lhy0403/TobiiMouse.svg)](http://hits.dwyl.io/lhy0403/TobiiMouse)

Make tobii EyeTracker act like a mouse.


Travis CI: [![Build Status](https://travis-ci.com/lhy0403/TobiiMouse.svg?branch=master)](https://travis-ci.com/lhy0403/TobiiMouse)

AppVeyor: [![Build status](https://ci.appveyor.com/api/projects/status/06yq9v21s6b619nh/branch/master?svg=true)](https://ci.appveyor.com/project/lhy0403/tobiimouse/branch/master)

## Dependencies
- Coming soon...

## Build
### For Linux:
```bash
# Clone the repository.
git clone https://github.com/lhy0403/TobiiMouse
cd ./TobiiMouse/src/
mkdir build && cd build

# QMake and make
qmake ../
make

# Done, if everything goes well, you should have successfully built it.
```
### For Windows:
```cmd
REM Coming soon!
```
## USE
+ **Trace Gaze point**. Automatically trace user's eyes gaze point, then move the cursor to the address.
+ **Blink Events**.	
	+ Single left blink----->Mouse LEFT HIT.
	+ Single right blink----->Open up or Turn off *Head Events Response*.
+ **Head Events**.
open action
	+ Turn left-----> Keyboard "A".
	+ Turn right -----> Keyboard "D".
	+ Rise up------> Keyboard "W".
	+ Nod ------> Keyboard "S".