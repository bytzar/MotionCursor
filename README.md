# MotionCursor
This program maps gyro inputs from an SDL3 supported controller to the Windows cursor, as well as the EAST button to left click. The intended usage is for QoL Splatoon on CEMU, enabling you to sit back and relax, without the need to jankily lift your hand to reach for your mouse in order to superjump every death (it bugged me crazy). Now you wont need to take your hands of your controler mid match.

Other CEMU or 3DS games even may profit from this as well. Say Wind Waker for example. Sure, you still have to pause, but this is still way directer than navigating that inventory with sticks.
This may have some extended use cases for laptops plugged into TVs for example, so you needn't get up or have keyboard and mice next to you.
It may even serve accesibility purposes. Should there be such need reach out so I'll make it have some more QoL improvements like launch on startup.

# Usage
- Download latest release
- Unzip
- Place your controller on a flat surface
- Run the MotionCursor.exe
- Wait for calibration to finsish
- Ensure the screenshot button is mapped to show gamepad view in cemu
- Ensure the toggle checkbox is not checked
- Ensure you don't have the gamepad and TV screen displayed at the same time

- Hold down the Screenshot button to activate functionality. Once release functionality is disabled, allowing your cursor to hide. Once reactivated it centers the cursor before moving it.

This was designed with the Nintendo Switch pro conroller in mind, but should work with others as well. While the screenshot button is held down, gyro inputs will be mapped to the mouse cursor and the A button (East) to left mouse click. The A button can be held down for holding the left click. Both will release parallely. Additionaly, before gyro data is mapped to the cursor it is reset to the middle of the screen.

# Compile yourself
Have git installed.

- Run `git clone --recursive https://github.com/bytzar/MotionCursor.git` in a terminal window 
- open motioncursor.sln
- compile main.cpp


# Roadmap

- Customizeable input mapping
- Toggle Support
- GUI
- Improved calibration
- Fail saves
- More options
- Multi monitor support
- Multi controller support
- Mapping home button to ctrl + Tap (Enables having both hold and toggle for Gamepadview. Useful for Splatoon where you may need toggle functionality in queue but hold mid match.)
- Record cursor position to shortcut frequent actions (e.g. map super jump to dpad)
- Right stick for percise cursor adjustments
- Proper exit
- Polish (refractoring, clean up, etc.)
