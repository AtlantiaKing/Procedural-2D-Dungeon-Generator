//-----------------------------------------------------------------
// Main Game  File
// C++ Header - DungeonGeneratorMain.h - version v7_01					
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Resource.h"	
#include "GameEngine.h"
#include "AbstractGame.h"
#include "Dungeon.h"

//-----------------------------------------------------------------
// DungeonGeneratorMain Class																
//-----------------------------------------------------------------
class DungeonGeneratorMain : public AbstractGame, public Callable
{
public:				
	//---------------------------
	// Constructor(s) and Destructor
	//---------------------------
	DungeonGeneratorMain();

	virtual ~DungeonGeneratorMain();

	//---------------------------
	// Disabling copy/move constructors and assignment operators   
	//---------------------------
	DungeonGeneratorMain(const DungeonGeneratorMain& other) = delete;
	DungeonGeneratorMain(DungeonGeneratorMain&& other) noexcept = delete;
	DungeonGeneratorMain& operator=(const DungeonGeneratorMain& other) = delete;
	DungeonGeneratorMain& operator=(DungeonGeneratorMain&& other) noexcept = delete;

	//---------------------------
	// General Methods
	//---------------------------
	void Initialize(HINSTANCE hInstance) override;
	void Start() override;
	void End() override;
	void MouseButtonAction(bool isLeft, bool isDown, int x, int y, WPARAM wParam) override;
	void MouseWheelAction(int x, int y, int distance, WPARAM wParam) override;
	void MouseMove(int x, int y, WPARAM wParam) override;
	void CheckKeyboard() override;
	void KeyPressed(TCHAR cKey) override;
	void Paint(RECT rect) override;
	void Tick() override;
	
	void CallAction(Caller* callerPtr) override;

private:
	// -------------------------
	// Datamembers
	// -------------------------
	std::unique_ptr<Dungeon> m_pDungeon{};
	std::unique_ptr<CheckBox> m_pSlowGenerateCheckBox{};
	std::unique_ptr<Button> m_pRegenerateButton{};
	std::unique_ptr<TextBox> m_pSeedTextBox{};
	std::unique_ptr<TextBox> m_pInitRadiusTextBox{};
	std::unique_ptr<TextBox> m_pInitRoomCountTextBox{};
	std::unique_ptr<TextBox> m_pNrKeysTextBox{};
	std::unique_ptr<CheckBox> m_pBossKeyCheckBox{};

	Vector2 m_PrevMousePos{};
};
