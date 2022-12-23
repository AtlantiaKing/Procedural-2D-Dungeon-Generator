//-----------------------------------------------------------------
// Main Game File
// C++ Source - DungeonGeneratorMain.cpp - version v7_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "DungeonGeneratorMain.h"																				

//-----------------------------------------------------------------
// DungeonGeneratorMain methods																				
//-----------------------------------------------------------------
DungeonGeneratorMain::DungeonGeneratorMain()
{
	// nothing to create
}

DungeonGeneratorMain::~DungeonGeneratorMain()
{
	// nothing to destroy
}

void DungeonGeneratorMain::Initialize(HINSTANCE hInstance)
{
	// Set the required values
	AbstractGame::Initialize(hInstance);
	GAME_ENGINE->SetTitle(_T("Procedular 2D Dungeon Generator"));					
	GAME_ENGINE->RunGameLoop(true);		
	
	// Set the optional values
	GAME_ENGINE->SetWidth(1024);
	GAME_ENGINE->SetHeight(768);
    GAME_ENGINE->SetFrameRate(50);

	// Set the keys that the project needs to listen to
	//tstringstream buffer;
	//buffer << _T("KLMO");
	//buffer << (TCHAR) VK_LEFT;
	//buffer << (TCHAR) VK_RIGHT;
	//GAME_ENGINE->SetKeyList(buffer.str());
}

void DungeonGeneratorMain::Start()
{
	// New seed for the random positions
	srand(time(NULL));

	// Create dungeon
	m_pDungeon = std::make_unique<Dungeon>();
	m_pDungeon->SetRoomBounds(4, 40);
	m_pDungeon->GenerateDungeon();
}

void DungeonGeneratorMain::End()
{
	// Insert the code that needs to be executed at the closing of the project
}

void DungeonGeneratorMain::MouseButtonAction(bool isLeft, bool isDown, int x, int y, WPARAM wParam)
{	
	// Insert the code that needs to be executed when the project registers a mouse button action
}


void DungeonGeneratorMain::MouseWheelAction(int x, int y, int distance, WPARAM wParam)
{	
	// Insert the code that needs to be executed when the project registers a mouse wheel action
}

void DungeonGeneratorMain::MouseMove(int x, int y, WPARAM wParam)
{	
	// Insert the code that needs to be executed when the mouse pointer moves across the project window
}

void DungeonGeneratorMain::CheckKeyboard()
{	
	// Here you can check if a key of choice is held down
	// Is executed once per frame if the Project Loop is running 
}

void DungeonGeneratorMain::KeyPressed(TCHAR cKey)
{	
	// DO NOT FORGET to use SetKeyList() !!

	// Insert the code that needs to be executed when a key of choice is pressed
	// Is executed as soon as the key is released
	// You first need to specify the keys that the research engine needs to watch by using the SetKeyList() method
}

void DungeonGeneratorMain::Paint(RECT rect)
{
	m_pDungeon->Draw();
}

void DungeonGeneratorMain::Tick()
{
	// Insert non-paint code that needs to be executed each tick 
}

void DungeonGeneratorMain::CallAction(Caller* callerPtr)
{
	// Insert the code that needs to be executed when a Caller has to perform an action
}
