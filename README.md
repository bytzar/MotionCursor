<p align="center">
  <a href="https://ko-fi.com/baehrei" target="_blank">
    <img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Buy Me a Coffee at ko-fi.com">
  </a>
</p>

<p align="center">
  <a href="https://img.shields.io/github/downloads/bytzar/MotionCursor/total.svg" target="_blank">
    <img src="https://img.shields.io/github/downloads/bytzar/MotionCursor/total.svg" alt="Total downloads">
  </a>
</p>

# MotionCursor
Control your mouse with your controller! MotionCurosr reads your controller's gyro data to offset your cursors position and simulate left clicks. No longer are you chained to a mouse and keyboard, when you just want to sit back and relax playing games with your controller. This program fills a hole in Wii U emulation, enabling you to fully enjoy the benefits of the touchscreen without having to reach for your mouse. But not just in Wii U emulation, 3Ds, Bluestacks or just plain when you have your laptop plugged into a TV the chains are off. Control Windows now with your controller on your couch, with your chair leened back. Break the input triangle into a focused dot.

# How To
![image](https://github.com/user-attachments/assets/f523a74b-7fb4-4611-b0f3-a0495912ad6e)
If your controller is not already connected on launch, hit `Refresh` and check the `Controllers` button.
- place your controller on a flat surface and hit `calibrate`
- calibration is swift, the ui will let you know once it has finished
- an activation button is used to control when the cursor should be moved based on gyro, this functionality can be adjusted to be a toggle or to not be required
- hit the `remap activation button` and the press the desired button on your controller
- whenever that button is held down, functionality will be active
- for cemu users it is recommended to map the same button used to display the gamepad screen and to also copy whether you set up toggle or hold
- then choose which button shall emulate a left click vi `remap click button`

# Features
- MotionCursor is not bound to your Windows cursor, as it emulates mouse movements. This means, you can use this in games which disable the cursor (e.g. Minecraft). So you could play it like how the camera controls in Splatoon.
- You can map a `lock` and a `reset` button
- `reset` positions the cursor in the center of your primary monitor
- `lock` disables gyro mapping in cases where you would want to repostion your hand without moving the camera
- additionaly, it can be tough navigating windows when the mouse is jittery, so you can lock it and then click easily
- when the program is exited properly all settings are saved to `MotionCursor.ini`, next to theexe. If you had used multiple controllers and calibrated them, the last calibration will be written to file

# Roadmap
Depending on the brnach, following functionality is to come:
- offsetting your mouse with control sticks (b: stickcursor)
- mapping controller inputs to keyboard keys (b: ?)
- hiding theprogram to systray (b: ?)
- running it as a startup program (b: ?)

# Branches
- master, the lightest version, dedicated to complementing other games, and your comfort
- stickcursor, implement more sophisticated mouse and keyboard emulation with the controller, dedicated to playing other games with this new control scheme,  and more accessibility use cases
- macros, a dead branch which had awful bugs BUT technically functional, let you record a position on screen and via hotkey position your cursor there and click quickly
- dev, dev

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
- The relevant files are `main.cpp`, `main_gui.cpp` and `main_gui.h` (latter may be buried under external dependencies)

# SEO
Use controller for cursor windows, use controller as mouse, control mouse with controller, move cursor with controller, gamepad mouse Windows, Windows controller mouse tool, controller as mouse Windows 10, controller as mouse Windows 11, map controller to mouse, joystick mouse emulator, turn gamepad into mouse, use PS5 controller as mouse, use Switch Pro controller as mouse, use Xbox controller to move mouse, use controller to move cursor, mouse control with game controller, game controller cursor control, controller mouse movement tool, motion controls as mouse, gyro mouse Windows, gyro aim Windows, use controller gyro for mouse, motion control mouse emulator, Steam Deck gyro mouse Windows, PS5 gyro mouse PC, map gyro to mouse movement, gyroscope to mouse cursor, use motion controls on PC, controller gyro mouse support, move mouse with joystick, convert gamepad to mouse, joystick to mouse mapper, use controller as touchpad, emulate mouse with controller, controller pointer movement, aim with controller gyro, Windows gyro aiming, motion aiming on PC, how to use controller to move mouse, how to control mouse with controller, mouse with gamepad on Windows, best controller mouse software, lightweight controller mouse tool, controller mouse alternative to Steam Input, controller mouse for non-Steam games
