//---------------------------
// Includes
//---------------------------
#include "Dungeon.h"
#include "DungeonGenerator.h"
#include "DungeonSolver.h"

//---------------------------
// Member functions
//---------------------------
void Dungeon::GenerateDungeon()
{
	m_Generator.GenerateDungeon(m_Rooms);

	DungeonSolver solver{ this };

	bool dungeonIsCorrect{ solver.Solve() };
}

void Dungeon::Update()
{
	m_Generator.Update(m_Rooms);
}

int Dungeon::GetStartRoom() const
{
	for (int i{}; i < m_Rooms.size(); ++i)
	{
		if (m_Rooms[i].GetRoomType() == DungeonRoom::DungeonRoomType::Start)
			return i;
	}
	return -1;
}

int Dungeon::GetEndRoom() const
{
	for (int i{}; i < m_Rooms.size(); ++i)
	{
		if (m_Rooms[i].GetRoomType() == DungeonRoom::DungeonRoomType::End)
			return i;
	}
	return -1;
}

Vector2 Dungeon::GetRoomPositionFromIndex(int roomIdx)  const
{
	return m_Rooms[roomIdx].GetPosition() + m_Rooms[roomIdx].GetSize() / 2;
}

const std::vector<int>& Dungeon::GetRoomConnectionsFromIndex(int roomIdx)  const
{
	return m_Rooms[roomIdx].GetConnections();
}

void Dungeon::Draw() const
{
	for (const DungeonRoom& room : m_Rooms)
	{
		room.Draw();
	}

	if (!m_Generator.IsDone())
	{
		m_Generator.RenderDebug();
	}
}
