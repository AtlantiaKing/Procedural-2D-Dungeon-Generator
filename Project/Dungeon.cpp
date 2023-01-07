//---------------------------
// Includes
//---------------------------
#include "Dungeon.h"
#include "DungeonGenerator.h"

//---------------------------
// Member functions
//---------------------------
void Dungeon::GenerateDungeon()
{
	m_Generator.GenerateDungeon(m_Rooms);
}

void Dungeon::Update()
{
	m_Generator.Update(m_Rooms);
}

Vector2 Dungeon::GetRoomPositionFromIndex(int roomIdx)  const
{
	return m_Rooms[roomIdx].GetPosition();
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
