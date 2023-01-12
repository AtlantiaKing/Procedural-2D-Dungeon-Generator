#include "SlowDungeonSolver.h"
#include "Dungeon.h"
#include "GameDefines.h"
#include "Camera.h"

SlowDungeonSolver::SlowDungeonSolver(std::shared_ptr<Dungeon> dungeon)
	: DungeonSolver{ dungeon }
{
}

bool SlowDungeonSolver::Solve(bool saveShortestRoute)
{
	// Reset the previous rooms and discovered rooms
	m_PreviousRooms = std::stack<int>();
	m_DiscoveredRooms.clear();
	m_TotalPath.clear();
	m_NrKeys = 0;
	m_ForcedPath.clear();

	// Set the current room to the start room
	m_CurRoom = m_pDungeon->GetStartRoom();

	m_IsActive = true;

	return true;
}

void SlowDungeonSolver::Update(float elapsedSec)
{
	// If the solver is not active, return
	if (!m_IsActive) return;
	// If the solver has finished the dungeon, return
	if (m_CurRoom == m_pDungeon->GetEndRoom()) return;

	m_AccuSec += elapsedSec;
	// After x seconds, solve a step of the dungeon
	if (m_AccuSec >= m_TimePerSec)
	{
		m_AccuSec -= m_TimePerSec;
		SolveStep();
	}
}

void SlowDungeonSolver::Disable()
{
	m_IsActive = false;
}

void SlowDungeonSolver::Draw() const
{
	// If the solver is not active, return
	if (!m_IsActive) return;

	// Calculate the position and size of the solver
	const Vector2 curPos{ CAMERA->ScalePoint(m_pDungeon->GetRoomPositionFromIndex(m_CurRoom)) };
	const int curSize{ CAMERA->ScaleSize(m_Size) };

	// Draw the dungeon solver
	GAME_ENGINE->SetColor(m_Color.GetColor());
	GAME_ENGINE->FillOval(curPos.x - curSize / 2, curPos.y - curSize / 2, curSize, curSize);

	// The position to draw keys
	const Vector2 keysStartPos{ curPos + Vector2{ 0, curSize } };
	// Draw the keys next to each other
	GAME_ENGINE->SetColor(RGB(255, 255, 0)); // Yellow
	for (int i{}; i < m_NrKeys; ++i)
	{
		GAME_ENGINE->DrawLine(keysStartPos.x + i * (curSize / 2), keysStartPos.y, keysStartPos.x + i * (curSize / 2), keysStartPos.y + curSize);
	}
}
