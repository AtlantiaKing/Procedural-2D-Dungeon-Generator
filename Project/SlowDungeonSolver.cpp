#include "SlowDungeonSolver.h"
#include "Dungeon.h"
#include "GameDefines.h"

SlowDungeonSolver::SlowDungeonSolver(Dungeon* dungeon)
	: DungeonSolver{ dungeon }
{
}

bool SlowDungeonSolver::Solve()
{
	// Reset the previous rooms and discovered rooms
	m_PreviousRooms = std::stack<int>();
	m_DiscoveredRooms.clear();

	// Set the current room to the start room
	m_CurRoom = m_pDungeon->GetStartRoom();

	m_IsActive = true;

	return true;
}

void SlowDungeonSolver::Update(float elapsedSec)
{
	if (m_CurRoom == m_pDungeon->GetEndRoom()) return;

	m_AccuSec += elapsedSec;
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
	if (!m_IsActive) return;

	const Vector2 curPos{ m_pDungeon->GetRoomPositionFromIndex(m_CurRoom) };

	GAME_ENGINE->SetColor(m_Color.GetColor());
	GAME_ENGINE->FillOval(curPos.x - m_Size / 2, curPos.y - m_Size / 2, m_Size, m_Size);
}
