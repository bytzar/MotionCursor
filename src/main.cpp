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
bool calibrated = false;


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

int dataRate = 200;

bool guiRecordingMacro;

bool isActivation = false;

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
        //müsste nicht mehr necessarySDL_PumpEvents(); //darf hier nicht sein, nur on main tread SOWOHL pump events als auch Polleevent müssen in main alle 200oder was auch immer datarate zentral gepullt werden

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
	calibrated = true;
	}
	std::cout << "\ncal thread deactive ";
}

bool isPressedMain(Macro* pMakro, SDL_Event pEvent) //zurück in klasse, hatte nicht damit zu tun also zurück wenn auch immer ich bopck hab
{
	//(*pMakro).isDown = (SDL_GetGamepadButton(activeCon, (*pMakro).buttonMac) && !(*pMakro).triggerMacro) || (SDL_GetGamepadAxis(activeCon, (*pMakro).axisMac) && (*pMakro).triggerMacro && SDL_GetGamepadAxis(activeCon, (*pMakro).axisMac) > 16000);
	(*pMakro).isDown = false;
	if (pEvent.type == SDL_EVENT_GAMEPAD_AXIS_MOTION)
	{
		(*pMakro).isDown = ((pEvent.gaxis.axis == (*pMakro).axisMac) && (*pMakro).triggerMacro && (SDL_GetGamepadAxis(activeCon, (*pMakro).axisMac) > 16000)); //
	}
	else if (pEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
	{
		(*pMakro).isDown = (static_cast<SDL_GamepadButton>(pEvent.gbutton.button) == (*pMakro).buttonMac) && !(*pMakro).triggerMacro;
	}
	std::cout << "\n TOKEN " << (*pMakro).isDown;
	return (*pMakro).isDown; //warum wid manchmal doppelt recorded?
}

void isSelftContraned() //isSelftContraned
{
	SDL_Event event;
	while (checkMacrosbool) {
		auto frameStart = std::chrono::high_resolution_clock::now();

		// Wait up to 10ms for an event. If nothing comes, it returns false.
		while (SDL_WaitEventTimeout(&event, 10)) {
			for (auto& macro : macros) {
				if (!macro.isDown) {
					if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && !macro.triggerMacro) {
						if (macro.buttonMac == static_cast<SDL_GamepadButton>(event.gbutton.button)) {
							macroRunningFuckyouUpdatLoop = true;
							SDL_Delay(20);
							ReplayMacro2(&macro, false);
							SDL_Delay(20);
							macroRunningFuckyouUpdatLoop = false;
						}
					}
					else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && macro.triggerMacro) {
						if (event.gaxis.axis == macro.axisMac && event.gaxis.value > 16000) {
							macroRunningFuckyouUpdatLoop = true;
							SDL_Delay(20);
							ReplayMacro2(&macro, false);
							SDL_Delay(20);
							macroRunningFuckyouUpdatLoop = false;
						}
					}
				}
			}
		}

		// You can still throttle further if needed
		auto frameEnd = std::chrono::high_resolution_clock::now();
		auto elapsed = frameEnd - frameStart;
		const std::chrono::milliseconds frameDuration(1000 / 100);
		if (elapsed < frameDuration) {
			std::this_thread::sleep_for(frameDuration - elapsed);
		}
	}

}

void CheckMacros() //CheckMacros
{
	//const std::chrono::microseconds cycleDuration2(1'000'000 / (dataRate)); und schießen muss funktionieren und console muss weg DAS HIER ALS NÄCHSTES OPTIMIZATION; DANN SAVE SETTINGS ODER NE DAVOR NOCH	DIESE CHECKBOXEN NE DAVOR NOCH DAS MAN MACROS LÖSCHEN KANN UND INPUT RESET UND DANN DEN DEBUG KNOPF SCHÖN ZU NEM RECORD MACRO KNOPF MACHEN
	const int targetFPS = 100;
	const std::chrono::milliseconds frameDuration4(1000 / targetFPS);
	while (checkMacrosbool) //terminiert einfach
	{
		auto frameStart4 = std::chrono::high_resolution_clock::now();
		if (true) //ey kein bock mehr so wetten beim löschen ich mach was kaputt so scheiß drauf einfach da lassen
		{
			//auto cycleStart = std::chrono::high_resolution_clock::now();
			//std::cout << "\ni am verbose";
			if (SDL_WaitEvent(&event))
			{
				if ((isActivation || NoReqAcMacro) && !NoMacros)
				{
					if (macros.size() > 0)
					{
						for (int i = 0; i < macros.size(); i++) //neue fuktion und ddan´´´threaden
						{
							std::cout << "\ni am super verbose";
							if (!macros[i].isDown)
							{
								std::cout << "\ni am super DUPER verbose";
								if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && !macros[i].triggerMacro)
								{
									if (macros[i].buttonMac == static_cast<SDL_GamepadButton>(event.gbutton.button))
									{
										std::cout << "\ni am mega verbose";
										//macros[i].isDown = true;
										macroRunningFuckyouUpdatLoop = true;
										SDL_Delay(20);
										ReplayMacro2(&macros[i], false); //gedrückt halten soll nur einmal sowie bei klick mit was down und nach replay freezed der weg
										SDL_Delay(20);
										macroRunningFuckyouUpdatLoop = false;
									}
								}
								else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && macros[i].triggerMacro)
								{
									if (event.gaxis.axis == macros[i].axisMac)
									{
										if (event.gaxis.value > 16000)
										{
											macroRunningFuckyouUpdatLoop = true;
											SDL_Delay(20);
											ReplayMacro2(&macros[i], false); //gedrückt halten soll nur einmal sowie bei klick mit was down und nach replay freezed der weg
											SDL_Delay(20);
											macroRunningFuckyouUpdatLoop = false;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		auto frameEnd4 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> elapsed4 = frameEnd4 - frameStart4;

		if (elapsed4 < frameDuration4) {
			std::this_thread::sleep_for(frameDuration4 - elapsed4);
		}
		 //CUURR das funktioniert nicht mit datarate ohne ist aber zu krass villeicht in update loop reinintigrieren ne nenenene problem ist das inputs nicht gecatched werden?
	}

	/*
	while (true) //for deb das hier hat ausch schuld am doppel macro record aber es gitb auch das man einfach mehrfach record drücken kann könnte sogar feature sein wenn ichs richtig mache
	{ //ich kann NCIHT überall wo ichs brauche einfach events pollen und lesen und was auch immer muss central passieren denke ich mal. einfach etwas was ka 200 mal die sekunde poll events auf public var events macht eun alle können AHHHHH WAS WENN DEQUED WIRD EIN EVENT NACH DEM LESEN, ja ist so aber lösung ist gleich 1X zentral das event für den frame oder so pollen und alle greifen auf das event zu 
		if (true) {
			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
				bool yrk = SDL_GAMEPAD_BUTTON_EAST == static_cast<SDL_GamepadButton>(event.gbutton.button);
				//isPressedMain(&macros[0], event); //
				std::cout << "Button ich bins: " << yrk << " (nenum value)\n";
			}
		}
	}
	*/
	

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

	const std::chrono::microseconds cycleDuration(1'000'000 / 200); //datarate aber ich cap glaub ich auch 200 oder mach es wenigstens einstellbar ne scheiß drauf ich mach 200 mehr braucht niemand
	//was wenn controller gewechselt wird, tesen könnte einfach klappen muss mit freund weil hab kein anderen vernünftig gyro der nicht joycon weirdness hat joycon hat in sdl ja eigene joycon gyro zeug nervig
	while (update) //muss immer true sein sonst terminated mainthread einfach und das schlimm!oh aber für interne zwekc könnte gut sein, für graceful shutdown zb müsste ich den loop beenden können -> nur mit variable einfach point blank kill execution ist auch nicht feine art//immer tru? bzw ist intended behaviour nicht dass es immer true ist. man soll ja mitten drinn nicht das ding pausieren können, wenn du das willst mach programm aus ist die idee. soll nicht so ein dummes program sein wo man es erst startet und dann muss man noch knopf drücken um funktionalität zu starten so wenn er nicht wollte hätte er program niht angemacht wenn er nicht mehr weill mach halt aus wa
	{
		auto cycleStart = std::chrono::high_resolution_clock::now();
		if (activeCon) //CHECKPOINT ok also hab das geschrieben weil ich dachte auf active con mit anderen thread zu schreiben während hier rennt macht proboeme mit deinitialisierung oder so jedenfalls villeicht activecon atomic machen baer das wahre problem war akku ist einfach tod gegangen muss gucken dass das registriert und so eigenlich hat program alles richtig gemacht keine crashes und so weiter (nicht viel getestet) nur muss dieses dropdown aktualisiert werden und active zu <none>
		{
			//SDL_PumpEvents();
			if ((((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) || NoReqAcGyrocursor) && !NoGyroCursor) //checks for activation button be pressed
			{
				if (!calibrated && NoReqAcGyrocursor)
				{
					Calibration();
				}
				isActivation = true;
				cursorPos.x = screenWidth / 2;
				cursorPos.y = screenHeight / 2;
				SetCursorPos(screenWidth / 2, screenHeight / 2);
				float data[2] = { 0.0f, 0.0f };
				while (((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) || NoReqAcGyrocursor)
				{
					while (macroRunningFuckyouUpdatLoop)
					{
						//trap the update thread without killing it because otherwise bug
					}
					SDL_PumpEvents(); //sonst nicht smooth

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

					if ((((SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick && SDL_GetGamepadAxis(activeCon, axisClick) > 16000)) && !wasDown) && !NoLeftClick)
					{
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						SendInput(1, inputs, sizeof(INPUT));
						wasDown = true;
					}
					else if (wasDown && (((!SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (!SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick))))
					{
						inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
						SendInput(1, inputs, sizeof(INPUT));
						wasDown = false;
					}
				}
			}
			isActivation = false;
			if ((((SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick && SDL_GetGamepadAxis(activeCon, axisClick) > 16000)) && !wasDown) && NoReqAcLeftClick && !NoLeftClick)
			{
				inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				SendInput(1, inputs, sizeof(INPUT));
				wasDown = true;
			}
			else if (wasDown && (((!SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (!SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick)) && NoReqAcLeftClick))
			{
				inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
				SendInput(1, inputs, sizeof(INPUT));
				wasDown = false;
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
	guiRecordingMacro = true;
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
	guiRecordingMacro = false;
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

	


	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listening = true;

	std::cout << "Waiting for button or trigger input... (7 seconds timeout)\n";

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (true) { // ehrlich einfah so lassen klappt ja ka SDL_WaitEventTimeout(&event, remaining_time) kann sein das probleme davon kommen das nicht auf main thread ist

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
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listeningClick = true;

	std::cout << "Waiting for button or trigger input... (7 seconds timeout)\n";

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (true) { //SDL_WaitEventTimeout(&event, remaining_time)

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
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listening = true; //alles auf listening machen später nur noch eine listening variable für alle obwohl mit <listenining> text in gui könnte probleme wir werden sehen, listening in macro wa oder ist doch so. 1x listening allgemien damit kein anderer anfängt und einmal spezifisch für gui
	(*pMacro).listening = true;

	std::cout << "Waiting for button or trigger input... (7 seconds timeout)\n";

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (true)
		{ //hoffen und beten SDL_WaitEventTimeout(&event, remaining_time)
			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				(*pMacro).buttonMac = static_cast<SDL_GamepadButton>(event.gbutton.button);
				std::cout << "Button pressed: " << (*pMacro).buttonMac << " (enum value)\n";
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
	(*pMacro).listening = false;
	theListeningOne = -1;
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