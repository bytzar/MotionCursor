#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_sensor.h>
#include <iostream>
#include <cmath>
#include <Windows.h>
#include <numeric>
#include "main_gui.h"


//TODO setting: Enable clicking even when motion activation is off
//update only every 1s/datarate



SDL_Gamepad* activeCon;

float avgDriftX;
float avgDriftY;


std::atomic<bool> runningCal = false;
std::atomic<bool> update = true;

std::thread runUpdateLoop;
 //atomic?
std::vector<const char*> controllers;
std::vector<int> conIds;

SDL_GamepadButton buttonActivator = SDL_GAMEPAD_BUTTON_MISC1; //default, settings in date ini machen aber gucken ka
SDL_GamepadButton buttonClick = SDL_GAMEPAD_BUTTON_EAST;

SDL_GamepadAxis axisActivator;
SDL_GamepadAxis axisClick;

bool listening = false;
bool listeningClick = false;
bool triggerAct = false;
bool triggerClick = false;

std::vector<int> macrosX;
std::vector<int> macrosY;

std::vector<Macro> macros;

std::thread runCheckMacros;

bool checkMacrosbool = true;
volatile bool macroRunningFuckyouUpdatLoop = false;

int dataRate = 999;


void Calibration()
{
	if (!runningCal && activeCon)
	{
	runningCal = true;
    float drift[2] = { 0.0f, 0.0f };
    float xDrift[10];
    float yDrift[10];
    for (int i = 0; i < 10; i++) //calibaration, takes 10 samples of idle gyro for avg and counters later TODO: needs feedack for end of calibration
    {
        SDL_PumpEvents();

        SDL_GetGamepadSensorData(activeCon, SDL_SENSOR_GYRO, drift, 2);
        xDrift[i] = drift[1];
        yDrift[i] = drift[0];

        std::cout << "\nx: " << xDrift[i] << "y: " << yDrift[i] << "\n";
        SDL_Delay(5); // auch beim letzten aber jucki TODO je nach datarate
		if (!runningCal) //muss ich aborten wenn das ganze keine 100ms dauert? ich mein je nach datarate aber trotzdem
		{
			std::cout << "abort";
			return;
		}
    }
    avgDriftX = std::accumulate(xDrift + 1, xDrift + 10, 0.0f) / 9.0f; //skips first value because it is 0 for ome reason. then divides by 9 as ve avergae 9 values
    avgDriftY = std::accumulate(yDrift + 1, yDrift + 10, 0.0f) / 9.0f;
	runningCal = false;
	}
	std::cout << "\ncal thread deactive ";
}

bool isPressedMain(Macro* pMakro) //zurück in klasse, hatte nicht damit zu tun also zurück wenn auch immer ich bopck hab
{
	(*pMakro).isDown = (SDL_GetGamepadButton(activeCon, (*pMakro).buttonMac) && !(*pMakro).triggerMacro) || (SDL_GetGamepadAxis(activeCon, (*pMakro).axisMac) && (*pMakro).triggerMacro && SDL_GetGamepadAxis(activeCon, (*pMakro).axisMac) > 16000);
	return (*pMakro).isDown;
}

void CheckMacros()
{
	const std::chrono::microseconds cycleDuration(1'000'000 / dataRate);
	while (checkMacrosbool)
	{
		auto cycleStart = std::chrono::high_resolution_clock::now();
		std::cout << "\ni am verbose";
		//SDL_PumpEvents(); //? wichitg? zu viel=?
		for (int i = 0; i < macros.size(); i++) //neue fuktion und ddan´´´threaden
		{
			std::cout << "\ni am super verbose";
			if (!macros[i].isDown)
			{
				if (isPressedMain(&(macros[i])))
				{
					std::cout << "\ni am mega verbose";
					macros[i].isDown = true;
					macroRunningFuckyouUpdatLoop = true;
					SDL_Delay(20);
					ReplayMacro2(&macros[i], false); //gedrückt halten soll nur einmal sowie bei klick mit was down und nach replay freezed der weg
					SDL_Delay(20);
					macroRunningFuckyouUpdatLoop = false;
				}
			}
			else if (!(isPressedMain(&(macros[i]))))
			{
				macros[i].isDown = false;
			}
		}
		auto cycleEnd = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(cycleEnd - cycleStart);

		// Sleep for the remaining time in the 5ms cycle
		if (elapsed < cycleDuration) {
			std::this_thread::sleep_for(cycleDuration - elapsed);
		}
		else {
			// (Optional) Handle overrun here
			std::cout << "unicode Cycle took too long! Dropping behind..." << std::endl;
		}
	}
}

void UpdateLoop()
{
    float data[2] = { 0.0f, 0.0f }; //Reset delta yaw and pitch for cursor
    float DYNdata[2] = { 0.0f, 0.0f }; //Reset read delta yaw and pitch

	int screenWidth = GetSystemMetrics(SM_CXSCREEN); //später wenn main in einzelnen aufgaben zerstückelt und rendermain wahrscheinlich zu main wird, machen, dass imgui window sized virtel des bildschirms groß ist. irgendwie wichtig wann man resolution hold weil manchmal dpi dependent manchaml independent sau komisch
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); //was wenn in benutzung bildschirm res oder dpi geändert wird............. neeeeeeeeeeeee macht keiner

	POINT cursorPos;
	INPUT inputs[1] = {};

	inputs[0].type = INPUT_MOUSE;
	bool wasDown = false;

	if (runCheckMacros.joinable())
	{
		checkMacrosbool = false;
		runCheckMacros.join();
	}
	checkMacrosbool = true;
	runCheckMacros = std::thread(CheckMacros);

	const std::chrono::microseconds cycleDuration(1'000'000 / dataRate);
	//was wenn controller gewechselt wird, tesen könnte einfach klappen muss mit freund weil hab kein anderen vernünftig gyro der nicht joycon weirdness hat joycon hat in sdl ja eigene joycon gyro zeug nervig
	while (update) //muss immer true sein sonst terminated mainthread einfach und das schlimm!oh aber für interne zwekc könnte gut sein, für graceful shutdown zb müsste ich den loop beenden können -> nur mit variable einfach point blank kill execution ist auch nicht feine art//immer tru? bzw ist intended behaviour nicht dass es immer true ist. man soll ja mitten drinn nicht das ding pausieren können, wenn du das willst mach programm aus ist die idee. soll nicht so ein dummes program sein wo man es erst startet und dann muss man noch knopf drücken um funktionalität zu starten so wenn er nicht wollte hätte er program niht angemacht wenn er nicht mehr weill mach halt aus wa
	{
		auto cycleStart = std::chrono::high_resolution_clock::now();
		if (activeCon) //CHECKPOINT ok also hab das geschrieben weil ich dachte auf active con mit anderen thread zu schreiben während hier rennt macht proboeme mit deinitialisierung oder so jedenfalls villeicht activecon atomic machen baer das wahre problem war akku ist einfach tod gegangen muss gucken dass das registriert und so eigenlich hat program alles richtig gemacht keine crashes und so weiter (nicht viel getestet) nur muss dieses dropdown aktualisiert werden und active zu <none>
		{
			SDL_PumpEvents();
			if ((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) //checks for activation button be pressed
			{
				cursorPos.x = screenWidth / 2;
				cursorPos.y = screenHeight / 2;
				SetCursorPos(screenWidth / 2, screenHeight / 2);
				float data[2] = { 0.0f, 0.0f };
				while ((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000))
				{
					while (macroRunningFuckyouUpdatLoop)
					{

					}
					SDL_PumpEvents();

					SDL_GetGamepadSensorData(activeCon, SDL_SENSOR_GYRO, DYNdata, 2);

					//subtracts drift
					DYNdata[1] -= avgDriftX;
					DYNdata[0] -= avgDriftY;

					const float threashold = 0.02f; //0.01f min maybe, maybe controller specific ()customizeable. automatic threshold. während calibration test values also gucken ob im bereich 000.0003 oder 0.0007 und je nachdem plus x prozent davon als threshold

					if (DYNdata[0] > threashold || DYNdata[0] < -threashold) //discard if movement is so minor it is probably drift
					{
						data[0] += DYNdata[0];
					}
					if (DYNdata[1] > threashold || DYNdata[1] < -threashold)
					{
						data[1] += DYNdata[1];
					}

					int nill = 1;
					if (data[1] < 0) { nill = -1; }

					int nuhull = 1;
					if (data[0] < 0) { nuhull = -1; }

					float MaxGyroForComforty = 125.0f / sensitivity;
					float MaxGyroForComfortx = 125.0f / sensitivity;

					if (data[1] >= MaxGyroForComforty) { data[1] = MaxGyroForComforty; }
					if (data[1] < -MaxGyroForComforty) { data[1] = -MaxGyroForComforty; }

					if (data[0] >= MaxGyroForComfortx) { data[0] = MaxGyroForComfortx; }
					if (data[0] < -MaxGyroForComfortx) { data[0] = -MaxGyroForComfortx; }

					float newposx = 0.0f;
					float newposy = 0.0f;

					if (!data[1] == 0)
					{
						newposx = (((abs(data[1]) / MaxGyroForComforty) * (float)(screenWidth / 2)) * nill);
					}
					if (!data[0] == 0)
					{
						newposy = (((abs(data[0]) / MaxGyroForComfortx) * (float)(screenHeight / 2)) * nuhull);
					}

					std::cout << "Gyro: X=" << data[0] << " Y=" << data[1] << std::endl;
					std::cout << "Gyro Delta: X=" << DYNdata[0] << " Delta Y=" << DYNdata[1] << std::endl;

					GetCursorPos(&cursorPos);

					cursorPos.y = -newposy + screenHeight / 2;
					cursorPos.x = -newposx + screenWidth / 2;

					// Move the mouse
					SetCursorPos(cursorPos.x, cursorPos.y);

					if (((SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick && SDL_GetGamepadAxis(activeCon, axisClick) > 16000)) && !wasDown)
					{
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						SendInput(1, inputs, sizeof(INPUT));
						wasDown = true;
					}
					else if (wasDown && ((!SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (!SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick)))
					{
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
						SendInput(1, inputs, sizeof(INPUT));
						wasDown = false;
					}
				}
			}
		}
		auto cycleEnd = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(cycleEnd - cycleStart);

		// Sleep for the remaining time in the 5ms cycle
		if (elapsed < cycleDuration) {
			std::this_thread::sleep_for(cycleDuration - elapsed);
		}
		else {
			// (Optional) Handle overrun here
			std::cout << "unicode Cycle took too long! Dropping behind..." << std::endl;
		}
	}
	std::cout << "\nupdate loop thread deactive ";
}



void UpdateCon(int pActiveConId)
{
	activeCon = SDL_OpenGamepad(pActiveConId); //brauche kem nicht einfach id plus 1, problem weil wir nur chars array rüber schicken aber mit reconnect und disconnect ich muss mit einher auch die kem ids schicken array mit zwei datentypen? oder... ne einfach noch ein array aber aus ints und einfach die kem werte in der richtigen reihenfolge. ungenutzte kems werden sowieso von sdl einfach geskiptt ja ok ez muss aber global lol so schlecht konvention
	std::cout << "\nupdate con thread deactive ";
}


void UpdateConList()
{
	
	if (activeCon)
	{
		SDL_CloseGamepad(activeCon);
	}
	update = false;
	if (runUpdateLoop.joinable())
	{
		runUpdateLoop.join();
	}
	checkMacrosbool = false;
	if (runCheckMacros.joinable())
	{
		runCheckMacros.join();
	}
	checkMacrosbool = true;
	SDL_Delay(200); //if refresh has issues 50% of the time WHAT DA FUCK; this line is only needed, now get this, when the conroller id is EVEN! WHY?! been looking for a race condition this wole time this is so stupid wasted my day. why would the evenness of an id matter its so stupid. its probably something else and the evenness is just a sympton 


	std::cout << "\ni am cout";
	if (SDL_Init(SDL_INIT_SENSOR | SDL_INIT_GAMEPAD) < 0) //Initializes controller and checks for available sensors, also checks for error
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		first = true;
		return;
	}


	int countControllers;
	SDL_JoystickID* gem = SDL_GetGamepads(&countControllers);

	controllers.resize(countControllers);
	conIds.resize(countControllers);

	if (countControllers == 0)
	{
		std::cout << "no Sdl controllers connected.";
		first = true;
		return;
	}

	SDL_JoystickID kem;
	for (int i = 0; i < countControllers; i++)
	{
		std::cout << "\nich war " << conIds[i];
		SDL_Gamepad* game = SDL_OpenGamepad(gem[i]);
		kem = SDL_GetGamepadID(game);
		conIds[i] = kem;
		controllers[i] = SDL_GetGamepadNameForID(kem);
		std::cout << "\njetzt bin ich " << conIds[i];
	}


	if (countControllers > 0)
	{
		//const char* name = SDL_GetGamepadNameForID(*gem);
		//std::cout << *gem << "\n " << name;

		//activeCon = SDL_OpenGamepad(1); //provisorisch ne doch auto select den ersten beim ersten mal falls oh... falls vorhanden
		//if (activeCon != SDL_OpenGamepad(conIds[0])) //gibt n racy cond aber ist doch complett im thread self contained oder nicht AAAAAAAAAA
		
			UpdateCon(conIds[0]);
			std::cout << "\nupdated mit " << conIds[0];
		
		

		//bool tf = SDL_GamepadConnected(activeCon);

		//std::cout << tf << " " << activeCon;

		SDL_SensorType type = SDL_SENSOR_GYRO;
		bool cem = SDL_GamepadHasSensor(activeCon, type); //add gyro joycon support
		std::cout << "\n\n\n" << cem;

		bool isenab = SDL_GamepadSensorEnabled(activeCon, type);
		std::cout << "\n isenab " << isenab;
		if (!isenab)
		{
			bool sen = SDL_SetGamepadSensorEnabled(activeCon, type, true);
			std::cout << "\n isenab now " << sen;
		}
		

		float rete = SDL_GetGamepadSensorDataRate(activeCon, type);
		dataRate = (int) rete;
		std::cout << "\n" << rete << "\n ";


		if (!SDL_GamepadSensorEnabled(activeCon, SDL_SENSOR_GYRO)) {
			std::cerr << "Gyroscope failed to enable!" << std::endl;
			//return;
		}
		update = true;
		runUpdateLoop = std::thread(UpdateLoop);
		std::cout << "\nupdate loop thread aktiv " << runUpdateLoop.get_id();
		std::cout << "\nupdate list thread deactive ";
	}
	first = true;
}

int main() //soon to be int init()
{
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SDL_SetHint("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "1");
	//UpdateConList();
	mainRender(NULL, nullptr);
	//std::thread guirenderer(mainRender, NULL, nullptr);
	//runUpdateLoop = std::thread(UpdateLoop);
    //Calibration();
	//UpdateLoop(); //ohne diese line ist prgram fert also main thread ist fertig deswegn crashen alle anderen threads einfach. kein problem wenn ich aus main einfach nur noch init methode mace
}
HHOOK mouseHook = NULL;  // Hook handle

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0 && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)) {
		MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
		std::cout << "Mouse clicked at: (" << mouseInfo->pt.x << ", " << mouseInfo->pt.y << ")\n";

		Macro mac;
		mac.cursorX = mouseInfo->pt.x;
		mac.cursorY = mouseInfo->pt.y;
		macros.push_back(mac);

		// Stop listening after the first click
		PostQuitMessage(0);
	}
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

void RecordMacro() {
	std::cout << "Waiting for next mouse click...\n";

	// Install the hook
	mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
	if (!mouseHook) {
		std::cerr << "Failed to install mouse hook!\n";
		return;
	}

	// Wait for a single click
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Uninstall the hook after the first click
	UnhookWindowsHookEx(mouseHook);
	mouseHook = NULL;
}

void ReplayMacro(int pPosX, int pPosY, bool pPreview)
{
	if (pPosX > 0 && pPosY > 0)
	{
		INPUT inputs[1] = {};
		SetCursorPos(pPosX, pPosY);
		if (!pPreview)
		{
			inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, inputs, sizeof(INPUT));
			inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, inputs, sizeof(INPUT));
		}
	}
}

void DEBUG()
{
	RecordMacro();
	//ReplayMacro(macrosX[0], macrosY[0]);
	std::cout << "removed all bugs";
}

void RemapActivator()
{
	/*
		update = false;
	if (runUpdateLoop.joinable())
	{
		runUpdateLoop.join();
	}		SDL_PumpEvents();	update = true;
	runUpdateLoop = std::thread(UpdateLoop); funktioniert nicht ohne klapps auch juckie
	*/

	

	SDL_Event event;
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listening = true;

	std::cout << "Waiting for button or trigger input... (7 seconds timeout)\n";

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (SDL_WaitEventTimeout(&event, remaining_time)) {

			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				buttonActivator = static_cast<SDL_GamepadButton>(event.gbutton.button);
				std::cout << "Button pressed: " << buttonActivator << " (enum value)\n";
				const char* buttonName = SDL_GetGamepadStringForButton(buttonActivator);
				triggerAct = false;
				std::cout << "Button pressed: " << (buttonName ? buttonName : "Unknown") << "\n";
				break;
			}
			else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
					if (event.gaxis.value > 16000) {
						axisActivator = static_cast<SDL_GamepadAxis>(event.gaxis.axis);
						triggerAct = true;
						std::string triggerName = (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "Left Trigger" : "Right Trigger";
						std::cout << triggerName << " pressed.\n";
						break;
					}
				}
			}
		}
	}
	//update loop in then cases starten
	std::cout << "Timeout reached or input detected.\n";
	listening = false;

}

void RemapClick()
{
	SDL_Event event;
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listeningClick = true;

	std::cout << "Waiting for button or trigger input... (7 seconds timeout)\n";

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (SDL_WaitEventTimeout(&event, remaining_time)) {

			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				buttonClick = static_cast<SDL_GamepadButton>(event.gbutton.button);
				std::cout << "Button pressed: " << buttonActivator << " (enum value)\n";
				const char* buttonName = SDL_GetGamepadStringForButton(buttonActivator);
				std::cout << "Button pressed: " << (buttonName ? buttonName : "Unknown") << "\n";
				triggerClick = false;
				break;
			}
			else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
					if (event.gaxis.value > 16000) {
						axisClick = static_cast<SDL_GamepadAxis>(event.gaxis.axis);
						triggerClick = true;
						std::string triggerName = (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "Left Trigger" : "Right Trigger";
						std::cout << triggerName << " pressed.\n";
						break;
					}
				}
			}
		}
	}
	//update loop in then cases starten
	std::cout << "Timeout reached or input detected.\n";
	listeningClick = false;
}

void RemapButton(Macro* pMacro)
{
	SDL_Event event;
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listening = true; //alles auf listening machen später nur noch eine listening variable für alle obwohl mit <listenining> text in gui könnte probleme wir werden sehen, listening in macro wa oder ist doch so. 1x listening allgemien damit kein anderer anfängt und einmal spezifisch für gui
	(*pMacro).listening = true;

	std::cout << "Waiting for button or trigger input... (7 seconds timeout)\n";

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (SDL_WaitEventTimeout(&event, remaining_time)) {

			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				(*pMacro).buttonMac = static_cast<SDL_GamepadButton>(event.gbutton.button);
				std::cout << "Button pressed: " << buttonActivator << " (enum value)\n";
				(*pMacro).buttonLable = SDL_GetGamepadStringForButton((*pMacro).buttonMac);
				(*pMacro).triggerMacro = false;
				break;
			}
			else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
					if (event.gaxis.value > 16000) {
						(*pMacro).axisMac = static_cast<SDL_GamepadAxis>(event.gaxis.axis);
						(*pMacro).triggerMacro = true;
						(*pMacro).buttonLable = (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "Left Trigger" : "Right Trigger";
						std::cout << (*pMacro).buttonLable << " pressed.\n";
						break;
					}
				}
			}
		}
	}
	//update loop in then cases starten
	std::cout << "Timeout reached or input detected.\n";
	listening = false;
	(*pMacro).listening = true;
}

void ReplayMacro2(Macro* pMacro, bool pPreview)
{
	if ((*pMacro).cursorX > 0 && (*pMacro).cursorY > 0)
	{
		INPUT inputs[1] = {};
		SetCursorPos((*pMacro).cursorX, (*pMacro).cursorY);
		if (!pPreview)
		{
			inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, inputs, sizeof(INPUT));
			inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, inputs, sizeof(INPUT));
		}
	}
}