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
	m_pDungeon->GenerateDungeon(-1);

	// Create UI
	m_pSlowGenerateCheckBox = std::make_unique<CheckBox>();
	m_pSlowGenerateCheckBox->SetBounds(GAME_ENGINE->GetWidth() - 50, GAME_ENGINE->GetHeight() - 40, 30);
	m_pSlowGenerateCheckBox->Show();
	m_pSlowGenerateCheckBox->AddActionListener(this);

	m_pRegenerateButton = std::make_unique<Button>(_T("Regenerate Dungeon"));
	m_pRegenerateButton->SetBounds(GAME_ENGINE->GetWidth() - 220, GAME_ENGINE->GetHeight() - 80, 200, 30);
	m_pRegenerateButton->Show();
	m_pRegenerateButton->AddActionListener(this);

	m_pSeedTextBox = std::make_unique<TextBox>();
	m_pSeedTextBox->SetBounds(GAME_ENGINE->GetWidth() - 220, GAME_ENGINE->GetHeight() - 120, 200, 30);
	m_pSeedTextBox->Show();
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

	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(_T("Slow Generation Enabled:"), GAME_ENGINE->GetWidth() - 230, GAME_ENGINE->GetHeight() - 18);
	GAME_ENGINE->DrawString(_T("Seed:"), GAME_ENGINE->GetWidth() - 265, GAME_ENGINE->GetHeight() - 98);
}

void DungeonGeneratorMain::Tick()
{
	// Insert non-paint code that needs to be executed each tick 
	m_pDungeon->Update();
}

void DungeonGeneratorMain::CallAction(Caller* callerPtr)
{
	// Insert the code that needs to be executed when a Caller has to perform an action
	if (callerPtr == m_pRegenerateButton.get())
	{
		// The seed for the generation
		int seed{ -1 };

		// Get the current seed from the textbox
		if (m_pSeedTextBox->GetText().size() > 0)
		{
			seed = std::stoi(m_pSeedTextBox->GetText());
			if (seed < 0) seed = -1;
		}

		// Generate the dungeon with the current seed
		m_pDungeon->GenerateDungeon(seed);
	}
	else if (callerPtr == m_pSlowGenerateCheckBox.get())
	{
		m_pDungeon->SetGenerationState(m_pSlowGenerateCheckBox->IsChecked());
	}
}
