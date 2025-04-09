[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/baehrei)


# Update
Due to issues, macros have been removed from master and the master release which significantly increased performance, stability and consistency. Macros still have a place as a general purpose utility, but not in the context of gaming. The last macro release can be found in release v1.0.

# MotionCursor
This program maps gyro inputs from an SDL3 supported controller to the Windows cursor, as well as the EAST button to left click. The intended usage is for QoL Splatoon on CEMU, enabling you to sit back and relax, without the need to jankily lift your hand to reach for your mouse in order to superjump every death (it bugged me crazy). Now you wont need to take your hands of your controler mid match. Cemu has now, on my release day, beat me to the punch by releasing a graphics pack mapping the dpad to superjumping ALTHOUGH I assume you cannot use it to jump to beacons so I've still got one up on them. Additionaly, thinking of Twilight princess HD, you could make a macro for the wolf human quick transform on the gamepad so I suppose I have 2+ up on cemu.

Other CEMU or 3DS games even may profit from this as well. Say Wind Waker for example. Sure, you still have to pause, but this is still way directer than navigating that inventory with sticks.
This may have some extended use cases for laptops plugged into TVs for example, so you needn't get up or have keyboard and mice next to you.
It may even serve accesibility purposes. Should there be such need reach out so I'll make it have some more QoL improvements like launch on startup.

# Usage
- Download latest release
- Unzip entire folder
- run 'MotionCursor.exe'
- It should automatically detect your controller, if it was connected before launch
- if not, click refresh to refresh controller list
- place your controller ona flat surface, and hit calibrate if expiriencing gyro drift
- remap the activation button to your liking
- when held down gyro mapping to cursor is active

- Hold down the Screenshot button to activate functionality. Once release functionality is disabled, allowing your cursor to hide. Once reactivated it centers the cursor before moving it.

This was designed with the Nintendo Switch pro conroller in mind, but should work with others as well. While the screenshot button is held down, gyro inputs will be mapped to the mouse cursor and the A button (East) to left mouse click. The A button can be held down for holding the left click (holding and dragging is implemented). Both will release parallely. Additionaly, before gyro data is mapped to the cursor it is reset to the middle of the screen.

# Compile yourself
Have git installed.
Have CMake installed.

- Create a new folder and open a Terminal window in that directory
- Run `git clone --recursive https://github.com/bytzar/MotionCursor.git`
- Run `cd MotionCursor`, `mkdir build` and `cd build`
- Run `cmake -G "Visual Studio 17 2022" ..` (Adjust according to IDE)
- Open newly created MotionCursor.sln in build
- You might have to set MotionCurser as Startup Project in the solution explorer
- Compile
