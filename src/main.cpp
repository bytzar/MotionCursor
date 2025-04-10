/*Notizblock
* controller calibrated lable safen, die neuen einstellungen safen sowie invert weil warum nicht. machen das calibrated on bootup stimmt
* und nicht zu not calibrated defaulete
* fick mein leben aber ich will machen das man keyboard und maus und so wie man will auf controller machen kann
* orgendeinen weg finden das computer spiele die auf wasd laufen joystick support haben
* óbwohl ich da nicht viel machen kann weil wasd ist halt einfach wasd. aber es könnte trotzdem klappen
* diagonal ist dann einfach wd gleichzeitig zum beipiel und rest kann man machen indem man mit gyro einfach winkel änder aber dann braucht man
* wie bei splatoon rechten stick erstmal und reset knopf
* aber manche sachen müssen in andere branches. zb rechter stick für cursor darf noch in master aber dieses auf keyboard mappen muss neuer branch
* und dieses reseten wenn neu gestartet wird das cursor in die mitte platziert wird muss optional sein weil wenn man windows navigiert kurz
* loslassen damit man vernünftig klicken kann und dann von da wieder weiter oder so ein cursor lock knopf um den kurz still zu halten wahrscheinlich
* besser weil dann hab ich kein problem mit reset on reset toggle weil villeicht will man so in ein spiel rein und dann wieder resetten nh
* dese´wegen cursor lock besser
* irgendeinelösunge dafür finden das self compiler wissen wonach sie suchen müssen. das maingui.h tief in external deps vergraben ist und main und maingui cpp die dateien sind.
* klar die können nachgucken aber ist nicht sehr intuitiv
*/

#pragma once

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <SDL3/SDL.h>
#include <SDL3/SDL_sensor.h>
#include <cmath>
#include <numeric>
#include "main_gui.h"

SDL_Gamepad* activeCon;
std::vector<const char*> controllers;
std::vector<int> conIds;

float avgDriftX;
float avgDriftY;
bool calibrated = false;
const char* calibratedConName = "Z";
std::atomic<bool> runningCal = false;

std::atomic<volatile bool> update = true;
std::thread runUpdateLoop;


SDL_GamepadButton buttonActivator = SDL_GAMEPAD_BUTTON_MISC1; 
SDL_GamepadButton buttonClick = SDL_GAMEPAD_BUTTON_EAST;
SDL_GamepadAxis axisActivator; //Triggers and buttons are different datatypes, so we prepare another variable in case triggers are used
SDL_GamepadAxis axisClick;
bool listening = false;
bool listeningClick = false;
bool triggerAct = false;
bool triggerClick = false;

//the datarate for a ns pro con. should be sufficient for all controllers capped at 200hz 
//because some controllers go a bit off the charts
int dataRate = 200; 

bool isActivation = false;
bool toggleWasDown = false;

//SDL gyro returns delta values. When the controller is on a flat surface it should return 0 on all axis. 
//If not 10 samples are taken, averaged and saved to always be subtracted from the polled delta values
void Calibration() 
{
	if (!runningCal && activeCon)
	{
	runningCal = true;
    float drift[2] = { 0.0f, 0.0f };
    float xDrift[10];
    float yDrift[10];
    for (int i = 0; i < 10; i++) //calibaration, takes 10 samples of idle gyro for avg and counters later
    {
        SDL_GetGamepadSensorData(activeCon, SDL_SENSOR_GYRO, drift, 2);
        xDrift[i] = drift[1];
        yDrift[i] = drift[0];

        SDL_Delay(20); 
		if (!runningCal) 
		{
			return;
		}
    }
	//skips first value because it is 0 for some reason. then divides by 9 as ve avergae 9 values
    avgDriftX = std::accumulate(xDrift + 1, xDrift + 10, 0.0f) / 9.0f; 
    avgDriftY = std::accumulate(yDrift + 1, yDrift + 10, 0.0f) / 9.0f;
	runningCal = false; //for threading
	calibrated = true; //for ui
	calibratedConName = SDL_GetGamepadNameForID(SDL_GetGamepadID(activeCon));
	}
}


void UpdateLoop()
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN); 
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	POINT cursorPos; //datatype for cursor position
	INPUT inputs[1] = {}; //var for simulating left click

	inputs[0].type = INPUT_MOUSE; //var for simulating left click
	bool wasDown = false; 

	const std::chrono::microseconds cycleDuration(1'000'000 / dataRate); //200hz
	while (update) 
	{
		auto cycleStart = std::chrono::high_resolution_clock::now();
		if (activeCon)
		{
			SDL_PumpEvents();
			//checks for activation button pressed. only run on te first frame of activator pressed
			if ((((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) || NoReqAcGyrocursor && isActivation) && !NoGyroCursor)
			{
				//bool toggle an
				//bool toggle was down an
				isActivation = true;
				toggleWasDown = true;
				screenWidth = GetSystemMetrics(SM_CXSCREEN);
				screenHeight = GetSystemMetrics(SM_CYSCREEN);
				cursorPos.x = screenWidth / 2; 
				cursorPos.y = screenHeight / 2;
				SetCursorPos(screenWidth / 2, screenHeight / 2); //reset cursor position to center of screen
				float data[2] = { 0.0f, 0.0f }; //Reset delta yaw and pitch for cursor. this is the total delta from the position from which the activator was pressed. an accumalation of all values read since the first activation so to say the current 2d state of the controller in space
				float DYNdata[2] = { 0.0f, 0.0f }; //Reset read delta yaw and pitch. this is only the momentary delta
				const std::chrono::microseconds cycleDurationU(1'000'000 / dataRate); //runs this loop only at 200hz to prevent cpu burn
				while ((((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) || (NoReqAcGyrocursor && isActivation)) && update)
				{
					if (!((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) && (NoReqAcGyrocursor && toggleWasDown))
					{
						toggleWasDown = false;
					}
					else if (((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) && (NoReqAcGyrocursor && !toggleWasDown))
					{
						isActivation = false;
						toggleWasDown = true;
						while (((SDL_GetGamepadButton(activeCon, buttonActivator) && !triggerAct) || (SDL_GetGamepadAxis(activeCon, axisActivator) && triggerAct && SDL_GetGamepadAxis(activeCon, axisActivator) > 16000)) && update)
						{ 
							SDL_Delay(200);
						}
						break;
					}
					auto cycleStartU = std::chrono::high_resolution_clock::now();
					SDL_PumpEvents(); //since gyro isnt an event (the while (pollevent) ignores it) and i run ui at a target of 15 fps we need to pump here to have proper 200hz in the update loop even though its not threadsafe but eh
					SDL_GetGamepadSensorData(activeCon, SDL_SENSOR_GYRO, DYNdata, 2); //read delta gyro data and place it in our delta array

					float data[2] = { 0.0f, 0.0f };
					//subtracts drift
					DYNdata[1] -= avgDriftX;
					DYNdata[0] -= avgDriftY;

					//inverts
					if (!invX)
					{
						DYNdata[1] *= -1;
					}
					if (!invY)
					{
						DYNdata[0] *= -1;
					}

					const float threashold = 0.02f; //as drift is not constant it is not zeroed out but very small. this threashold throws out deltas presumed drift

					if (DYNdata[0] > threashold || DYNdata[0] < -threashold) //discard if movement is so minor it is probably drift
					{
						data[0] = DYNdata[0]; //+=
					}
					if (DYNdata[1] > threashold || DYNdata[1] < -threashold)
					{
						data[1] = DYNdata[1];
					}

					//checks if values were negative as calculation calculates in absolutes
					int nill = 1;
					if (data[1] < 0) { nill = -1; }

					int nuhull = 1;
					if (data[0] < 0) { nuhull = -1; }

					//predetermined max movement that should be neccessary to go from 0 to edge of screen

					float MaxGyroForComforty = 40.0f / sensitivity;
					float MaxGyroForComfortx = 40.0f / sensitivity;

					//if at edge, do not add to data. this way you dont have to move all the way back to come back from the edge
					//if you moved way outside bounce you would normally hae to move the same distance back. this way you dont
					if (data[1] >= MaxGyroForComforty) { data[1] = MaxGyroForComforty; }
					if (data[1] < -MaxGyroForComforty) { data[1] = -MaxGyroForComforty; }

					if (data[0] >= MaxGyroForComfortx) { data[0] = MaxGyroForComfortx; }
					if (data[0] < -MaxGyroForComfortx) { data[0] = -MaxGyroForComfortx; }

					float newposx = 0.0f;
					float newposy = 0.0f;

					if (!data[1] == 0)
					{
						data[1] = (((abs(data[1]) / MaxGyroForComforty) * (float)(screenWidth / 2)) * nill);
					}
					if (!data[0] == 0)
					{
						data[0] = (((abs(data[0]) / MaxGyroForComfortx) * (float)(screenHeight / 2)) * nuhull);
					}

					GetCursorPos(&cursorPos);

					cursorPos.y = -newposy + screenHeight / 2;
					cursorPos.x = -newposx + screenWidth / 2;

					// Move the mouse
					//SetCursorPos(cursorPos.x, cursorPos.y);
					INPUT iput = { 0 };
					iput.type = INPUT_MOUSE;
					iput.mi.dx = data[1] * sensitivity;
					iput.mi.dy = data[0] * sensitivity;
					iput.mi.dwFlags = MOUSEEVENTF_MOVE; // relative move
					SendInput(1, &iput, sizeof(INPUT));

					//implemented this way to enable hold and drag functiponality
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
					auto cycleEndU = std::chrono::high_resolution_clock::now();
					auto elapsedU = std::chrono::duration_cast<std::chrono::microseconds>(cycleEndU - cycleStartU);

					// Sleep for the remaining time in the 5ms cycle
					if (elapsedU < cycleDurationU) {
						std::this_thread::sleep_for(cycleDurationU - elapsedU);
					}
					else {
					}
				}
			}
			isActivation = false;
			//in case do not require activation for left click is on
			if ((((SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick && SDL_GetGamepadAxis(activeCon, axisClick) > 16000)) && !wasDown) && NoReqAcLeftClick && !NoLeftClick)
			{
				inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				SendInput(1, inputs, sizeof(INPUT));
				wasDown = true;
			}
			else if (wasDown && (((!SDL_GetGamepadButton(activeCon, buttonClick) && !triggerClick) || (!SDL_GetGamepadAxis(activeCon, axisClick) && triggerClick))) && NoReqAcLeftClick)
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
	}
}



void UpdateCon(int pActiveConId) //sets all relevent variable to update the controller
{
	calibrated = false; //machen das beim ersten mal das nicht e´defaultet. odersagen calibriert auf folgenden controller ist besser denk ich dann ist das nur bei uncalibrated false und dann ist 0.0 auch ok für calibration weil wenn wir lable saven saven wir auch automatisch das calibrairt wurde überhauüt ez
	activeCon = SDL_OpenGamepad(pActiveConId);
	SDL_SensorType type = SDL_SENSOR_GYRO;
	bool cem = SDL_GamepadHasSensor(activeCon, type);

	bool isenab = SDL_GamepadSensorEnabled(activeCon, type);
	if (!isenab)
	{
		bool sen = SDL_SetGamepadSensorEnabled(activeCon, type, true);
	}


	float rete = SDL_GetGamepadSensorDataRate(activeCon, type);
	dataRate = (int)rete;


	if (!SDL_GamepadSensorEnabled(activeCon, SDL_SENSOR_GYRO)) {
	}
}


void UpdateConList()
{
	update = false;
	if (runUpdateLoop.joinable())
	{
		runUpdateLoop.join();
	} //terminate and wait fro the update loop thread jsut in case
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
	SDL_Delay(500); // give time for OS to clean up device handles
	SDL_InitSubSystem(SDL_INIT_GAMEPAD);
	//SDL_Delay(500); //if refresh has issues 50% of the time WHAT DA FUCK; this line is only needed, now get this, when the conroller id is EVEN! WHY?! been looking for a race condition this wole time this is so stupid wasted my day. why would the evenness of an id matter its so stupid. its probably something else and the evenness is just a sympton 


	if (SDL_Init(SDL_INIT_SENSOR | SDL_INIT_GAMEPAD) < 0) //Initializes controller and checks for available sensors, also checks for error
	{
		first = true;
		return; //i dont remember
	}


	int countControllers;
	SDL_JoystickID* gem = SDL_GetGamepads(&countControllers);

	controllers.resize(countControllers);
	conIds.resize(countControllers);

	if (countControllers == 0)
	{
		first = true;
		return;
	}

	SDL_JoystickID kem; //controller ids (even if the same controller) change on reinitilization so its not just a matter of counting up
	//the values can seem arbitrary at times 
	for (int i = 0; i < countControllers; i++)
	{
		SDL_Gamepad* game = SDL_OpenGamepad(gem[i]);
		kem = SDL_GetGamepadID(game);
		conIds[i] = kem;
		controllers[i] = SDL_GetGamepadNameForID(kem);
	}


	if (countControllers > 0) //if at least one controller initialize the first one whos id we have and start the updateloop
	{
		UpdateCon(conIds[0]);
		update = true;
		runUpdateLoop = std::thread(UpdateLoop);
	}
	first = true;
}

int main() //main thread reads settings and then does ui
{
	if (std::ifstream("MotionCursor.ini"))
	{
		std::ifstream file("MotionCursor.ini");
		
		int temporus;

		file >> fontSize;
		file >> avgDriftX;
		file >> avgDriftY;

		if (avgDriftX != 0 && avgDriftY != 0) //if controller has no drift then it will mistakenly think its not calibarated but no biggie
		{
			calibrated = true; //for ui purposes
		}

		file >> sensitivity;

		file >> temporus;
		buttonActivator = static_cast<SDL_GamepadButton>(temporus);

		file >> temporus;
		buttonClick = static_cast<SDL_GamepadButton>(temporus);

		file >> temporus;
		axisActivator = static_cast<SDL_GamepadAxis>(temporus);

		file >> temporus;
		axisClick = static_cast<SDL_GamepadAxis>(temporus);

		file >> triggerAct;
		file >> triggerClick;
		file >> NoReqAcLeftClick;
		file >> NoLeftClick;
		file >> NoGyroCursor;
	}
	SDL_SetHint("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "1");
	mainRender(NULL, nullptr);
	return 0;
}

void RemapActivator()
{
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listening = true;
	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (true) { // ehrlich einfah so lassen klappt ja ka SDL_WaitEventTimeout(&event, remaining_time) kann sein das probleme davon kommen das nicht auf main thread ist

			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				buttonActivator = static_cast<SDL_GamepadButton>(event.gbutton.button);
				const char* buttonName = SDL_GetGamepadStringForButton(buttonActivator);
				triggerAct = false;
				break;
			}
			else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
					if (event.gaxis.value > 16000) {
						axisActivator = static_cast<SDL_GamepadAxis>(event.gaxis.axis);
						triggerAct = true;
						std::string triggerName = (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "lefttrigger" : "righttrigger";
						break;
					}
				}
			}
		}
	}
	listening = false;

}
//code duplication, i could fix but realistically not so important
void RemapClick()
{
	const int timeout_ms = 7000; // 7 seconds
	Uint64 start_time = SDL_GetTicks();
	listeningClick = true;

	while (SDL_GetTicks() - start_time < timeout_ms) {

		int remaining_time = timeout_ms - (SDL_GetTicks() - start_time);
		if (true) { //SDL_WaitEventTimeout(&event, remaining_time)

			if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				buttonClick = static_cast<SDL_GamepadButton>(event.gbutton.button);
				const char* buttonName = SDL_GetGamepadStringForButton(buttonActivator);
				triggerClick = false;
				break;
			}
			else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
					if (event.gaxis.value > 16000) {
						axisClick = static_cast<SDL_GamepadAxis>(event.gaxis.axis);
						triggerClick = true;
						std::string triggerName = (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ? "lefttrigger" : "righttrigger";
						break;
					}
				}
			}
		}
	}
	listeningClick = false;
}
