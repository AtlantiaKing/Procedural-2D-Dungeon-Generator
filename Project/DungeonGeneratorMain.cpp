//-----------------------------------------------------------------
// Main Game File
// C++ Source - DungeonGeneratorMain.cpp - version v7_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "DungeonGeneratorMain.h"																				
#include "Camera.h"

//-----------------------------------------------------------------
// DungeonGeneratorMain methods																				
//-----------------------------------------------------------------
DungeonGeneratorMain::DungeonGeneratorMain()
{
	// nothing to create
}

DungeonGeneratorMain::~DungeonGeneratorMain()
{
	// Delete the camera singleton
	delete CAMERA;
}

void DungeonGeneratorMain::Initialize(HINSTANCE hInstance)
{
	// Set the required game engine values
	AbstractGame::Initialize(hInstance);
	GAME_ENGINE->SetTitle(_T("Procedular 2D Dungeon Generator"));					
	GAME_ENGINE->RunGameLoop(true);		
	
	// Set the optional game engine values
	GAME_ENGINE->SetWidth(1024);
	GAME_ENGINE->SetHeight(768);
    GAME_ENGINE->SetFrameRate(50);

	// Set the required camera values
	CAMERA->SetCenter({ GAME_ENGINE->GetWidth() / 2, GAME_ENGINE->GetHeight() / 2 });
}

void DungeonGeneratorMain::Start()
{
	// New seed for the random positions
	srand(static_cast<unsigned int>(time(NULL)));

	// Create dungeon
	m_pDungeon = std::make_shared<Dungeon>();
	m_pDungeon->GenerateDungeon();

	// Retrieve the generator from the dungeon
	const DungeonGenerator& generator{ m_pDungeon->GetGenerator() };

	// Create UI
	CreateUI(generator);

	// Create dungeon solver
	m_pDungeonSolver = std::make_unique<SlowDungeonSolver>(m_pDungeon);
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
	// The amount of zoom per distance
	constexpr float zoomMultiplier{ 0.1f };

	// The amount of zoom to add to the camera
	const float zoomValue{ static_cast<float>(distance) / abs(distance) * zoomMultiplier };

	// Add the zoom to the camera
	CAMERA->AddZoom(zoomValue);
}

void DungeonGeneratorMain::MouseMove(int x, int y, WPARAM wParam)
{	
	// When the right mouse button is held
	if ((wParam & MK_RBUTTON) == MK_RBUTTON)
	{
		// Move the camera according to the difference in mouse position
		CAMERA->Move(x - m_PrevMousePos.x, m_PrevMousePos.y - y);
	}

	// Store the previous mouse position
	m_PrevMousePos.x = x;
	m_PrevMousePos.y = y;
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
	// Draw the dungeon
	m_pDungeon->Draw();

	// Draw extra UI text
	GAME_ENGINE->SetColor(RGB(255, 255, 255));
	GAME_ENGINE->DrawString(_T("Slow Generation Enabled:"), GAME_ENGINE->GetWidth() - 230, GAME_ENGINE->GetHeight() - 58);
	GAME_ENGINE->DrawString(_T("Seed:"), GAME_ENGINE->GetWidth() - 134, GAME_ENGINE->GetHeight() - 98);
	GAME_ENGINE->DrawString(_T("Init Room Radius:"), GAME_ENGINE->GetWidth() - 214, GAME_ENGINE->GetHeight() - 138);
	GAME_ENGINE->DrawString(_T("Init Room Count:"), GAME_ENGINE->GetWidth() - 206, GAME_ENGINE->GetHeight() - 178);
	GAME_ENGINE->DrawString(_T("Key Count:"), GAME_ENGINE->GetWidth() - 169, GAME_ENGINE->GetHeight() - 218);
	GAME_ENGINE->DrawString(_T("Needs All Keys:"), GAME_ENGINE->GetWidth() - 163, GAME_ENGINE->GetHeight() - 258);

	GAME_ENGINE->SetColor(RGB(255, 0, 0));
	GAME_ENGINE->DrawString(m_ErrorMessage, 30, GAME_ENGINE->GetHeight() - 30);

	// Draw the dungeon solver
	m_pDungeonSolver->Draw();
}

void DungeonGeneratorMain::Tick(float elapsedSec)
{
	// Update the dungeon
	m_pDungeon->Update();

	// Update the dungeon solver
	m_pDungeonSolver->Update(elapsedSec);
}

void DungeonGeneratorMain::CallAction(Caller* callerPtr)
{
	if (callerPtr == m_pRegenerateButton.get())  // If the regenerate button is pressed
	{
		// Clear the error message
		m_ErrorMessage.clear();

		// Retrieve the generator from the dungeon
		DungeonGenerator& generator{ m_pDungeon->GetGenerator() };

		// The seed for the generation
		int seed{ -1 };

		// Get the current seed from the textbox
		if (!m_pSeedTextBox->GetText().empty())
		{
			try
			{
				seed = std::stoi(m_pSeedTextBox->GetText());
				if (seed < 0) seed = -1;
			}
			catch (const logic_error&)
			{
				// Display an error message
				m_ErrorMessage = _T("Couldn't read the seed textbox");
			}
		}

		// Set the seed of the dungeon generator
		generator.SetSeed(seed);

		// Get the current init radius from the textbox
		if (!m_pInitRadiusTextBox->GetText().empty())
		{
			try
			{
				const int radius{ std::stoi(m_pInitRadiusTextBox->GetText()) };
				if (radius > 0)
				{
					// Set the initial radius of the dungeon generator
					generator.SetInitialRadius(radius);
				}
				else
				{
					// Display an error message
					m_ErrorMessage = _T("Initial radius can't be or be less then 0");
				}
			}
			catch (const logic_error&)
			{
				// Display an error message
				m_ErrorMessage = _T("Couldn't read the initial radius textbox");
			}
		}

		// Get the current init room count from the textbox
		if (!m_pInitRoomCountTextBox->GetText().empty())
		{
			try
			{
				const int roomCount{ std::stoi(m_pInitRoomCountTextBox->GetText()) };
				if (roomCount > 0)
				{
					// Set the initial room count of the dungeon generator
					generator.SetInitialRoomCount(roomCount);
				}
				else
				{
					// Display an error message
					m_ErrorMessage = _T("Initial room count can't be or be less then 0");
				}
			}
			catch (const logic_error&)
			{
				// Display an error message
				m_ErrorMessage = _T("Couldn't read the initial room count textbox");
			}
		}

		// Get the current key count from the textbox
		if (!m_pNrKeysTextBox->GetText().empty())
		{
			try
			{
				const int keyCount{ std::stoi(m_pNrKeysTextBox->GetText()) };
				if (keyCount >= 0)
				{
					// Set the key count of the dungeon generator
					m_pDungeon->SetKeyCount(keyCount);
				}
				else
				{
					// Display an error message
					m_ErrorMessage = _T("Key count can't be less then 0");
				}
			}
			catch (const logic_error&)
			{
				// Display an error message
				m_ErrorMessage = _T("Couldn't read the key count textbox");
			}
		}

		// Apply the check boxes
		generator.SetGenerationState(m_pSlowGenerateCheckBox->IsChecked());
		m_pDungeon->SetNeedAllKeys(m_pNeedAllKeysCheckbox->IsChecked());

		// Generate the dungeon
		m_pDungeon->GenerateDungeon();

		// Disable the dungeno solver
		m_pDungeonSolver->Disable();
	}
	else if (callerPtr == m_pSolveDungeonButton.get())  // If the solve dungeon button is pressed
	{
		// Start the dungeon solver
		m_pDungeonSolver->Solve();
	}
}

void DungeonGeneratorMain::CreateUI(const DungeonGenerator& generator)
{
	// Regenerate button
	m_pRegenerateButton = std::make_unique<Button>(_T("Regenerate Dungeon"));
	m_pRegenerateButton->SetBounds(GAME_ENGINE->GetWidth() - 220, GAME_ENGINE->GetHeight() - 40, 200, 30);
	m_pRegenerateButton->Show();
	m_pRegenerateButton->AddActionListener(this);

	// Slow generation checkbox
	m_pSlowGenerateCheckBox = std::make_unique<CheckBox>();
	m_pSlowGenerateCheckBox->SetBounds(GAME_ENGINE->GetWidth() - 50, GAME_ENGINE->GetHeight() - 80, 30);
	m_pSlowGenerateCheckBox->Show();
	m_pSlowGenerateCheckBox->AddActionListener(this);

	// Seed textbox
	m_pSeedTextBox = std::make_unique<TextBox>();
	m_pSeedTextBox->SetBounds(GAME_ENGINE->GetWidth() - 90, GAME_ENGINE->GetHeight() - 120, 70, 30);
	m_pSeedTextBox->Show();

	// Initial radius textbox
	m_pInitRadiusTextBox = std::make_unique<TextBox>();
	m_pInitRadiusTextBox->SetBounds(GAME_ENGINE->GetWidth() - 90, GAME_ENGINE->GetHeight() - 160, 70, 30);
	m_pInitRadiusTextBox->Show();

	tstringstream initRadiusStream{};
	initRadiusStream << generator.GetInitialRadius();
	m_pInitRadiusTextBox->SetText(initRadiusStream.str());

	// Initial room count textbox
	m_pInitRoomCountTextBox = std::make_unique<TextBox>();
	m_pInitRoomCountTextBox->SetBounds(GAME_ENGINE->GetWidth() - 90, GAME_ENGINE->GetHeight() - 200, 70, 30);
	m_pInitRoomCountTextBox->Show();

	tstringstream initRoomCountStream{};
	initRoomCountStream << generator.GetInitialRoomCount();
	m_pInitRoomCountTextBox->SetText(initRoomCountStream.str());

	// Number keys textbox
	m_pNrKeysTextBox = std::make_unique<TextBox>();
	m_pNrKeysTextBox->SetBounds(GAME_ENGINE->GetWidth() - 90, GAME_ENGINE->GetHeight() - 240, 70, 30);
	m_pNrKeysTextBox->Show();
	m_pNrKeysTextBox->SetText(_T("0"));

	// Need all keys checkbox
	m_pNeedAllKeysCheckbox = std::make_unique<CheckBox>(true);
	m_pNeedAllKeysCheckbox->SetBounds(GAME_ENGINE->GetWidth() - 50, GAME_ENGINE->GetHeight() - 280, 30);
	m_pNeedAllKeysCheckbox->Show();

	// Solve dungeon button
	m_pSolveDungeonButton = std::make_unique<Button>(_T("Solve Dungeon"));
	m_pSolveDungeonButton->SetBounds(GAME_ENGINE->GetWidth() - 220, 20, 200, 30);
	m_pSolveDungeonButton->Show();
	m_pSolveDungeonButton->AddActionListener(this);
}
