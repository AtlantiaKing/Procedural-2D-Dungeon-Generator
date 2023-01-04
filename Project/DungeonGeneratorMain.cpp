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
	srand(static_cast<unsigned int>(time(NULL)));

	// Create dungeon
	m_pDungeon = std::make_unique<Dungeon>();
	m_pDungeon->GenerateDungeon();

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

	m_pInitRadiusTextBox = std::make_unique<TextBox>();
	m_pInitRadiusTextBox->SetBounds(GAME_ENGINE->GetWidth() - 220, GAME_ENGINE->GetHeight() - 160, 200, 30);
	m_pInitRadiusTextBox->Show();

	tstringstream initRadiusStream{};
	initRadiusStream << m_pDungeon->GetInitialRadius();
	m_pInitRadiusTextBox->SetText(initRadiusStream.str());

	m_pInitRoomCountTextBox = std::make_unique<TextBox>();
	m_pInitRoomCountTextBox->SetBounds(GAME_ENGINE->GetWidth() - 220, GAME_ENGINE->GetHeight() - 200, 200, 30);
	m_pInitRoomCountTextBox->Show();

	tstringstream initRoomCountStream{};
	initRoomCountStream << m_pDungeon->GetInitialRoomCount();
	m_pInitRoomCountTextBox->SetText(initRoomCountStream.str());
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
	GAME_ENGINE->DrawString(_T("Seed:"), GAME_ENGINE->GetWidth() - 264, GAME_ENGINE->GetHeight() - 98);
	GAME_ENGINE->DrawString(_T("Init Room Radius:"), GAME_ENGINE->GetWidth() - 344, GAME_ENGINE->GetHeight() - 138);
	GAME_ENGINE->DrawString(_T("Init Room Count:"), GAME_ENGINE->GetWidth() - 336, GAME_ENGINE->GetHeight() - 178);
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
			try
			{
				seed = std::stoi(m_pSeedTextBox->GetText());
				if (seed < 0) seed = -1;
			}
			catch (const logic_error&)
			{
				// TODO: Display an error message
			}
		}

		m_pDungeon->SetSeed(seed);

		// Get the current init radius from the textbox
		if (m_pInitRadiusTextBox->GetText().size() > 0)
		{
			try
			{
				const float radius{ std::stof(m_pInitRadiusTextBox->GetText()) };
				if (radius > 0)
				{
					m_pDungeon->SetInitialRadius(radius);
				}
				else
				{
					// TODO: Display an error message
				}
			}
			catch (const logic_error&)
			{
				// TODO: Display an error message
			}
		}

		// Get the current init room count from the textbox
		if (m_pInitRoomCountTextBox->GetText().size() > 0)
		{
			try
			{
				const int roomCount{ std::stoi(m_pInitRoomCountTextBox->GetText()) };
				if (roomCount > 0)
				{
					m_pDungeon->SetInitialRoomCount(roomCount);
				}
				else
				{
					// TODO: Display an error message
				}
			}
			catch (const logic_error&)
			{
				// TODO: Display an error message
			}
		}

		// Generate the dungeon
		m_pDungeon->GenerateDungeon();
	}
	else if (callerPtr == m_pSlowGenerateCheckBox.get())
	{
		m_pDungeon->SetGenerationState(m_pSlowGenerateCheckBox->IsChecked());
	}
}
