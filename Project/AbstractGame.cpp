//-----------------------------------------------------------------
// AbstractGame Object
// C++ Source - AbstractGame.cpp - version v7_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Resource.h"		// include file to be able to use resources
#include "GameEngine.h"		// include file to be able to use the game engine
#include "AbstractGame.h"

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//-----------------------------------------------------------------
// AbstractGame methods
//-----------------------------------------------------------------
void AbstractGame::Initialize(HINSTANCE hInstance)
{
	// Set required values
	GAME_ENGINE->SetTitle(_T("Game Engine version 7_01"));
	GAME_ENGINE->SetIcon(IDI_BIG);
	GAME_ENGINE->SetSmallIcon(IDI_SMALL);
	//GAME_ENGINE->RunGameLoop(true);

	// Set optional values
	GAME_ENGINE->SetWidth(640);
	GAME_ENGINE->SetHeight(480);
	GAME_ENGINE->SetFrameRate(50);
	//GAME_ENGINE->SetKeyList(String("QSDZ") + (TCHAR) VK_SPACE);
}