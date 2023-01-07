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

	const int startIdx{ GetStartRoom() };
	const int endIdx{ GetEndRoom() };

	std::vector<int> keyRooms{};
	std::vector<int> lockedRooms{};

	for (int i{}; i < m_NrKeys; ++i)
	{
		int prevKeyRoomIdx{ -1 };
		int prevDoorRoomIdx{ -1 };

		do
		{
			for (int roomIdx : keyRooms)
			{
				m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);
			}
			for (int roomIdx : lockedRooms)
			{
				m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);
			}

			if (prevKeyRoomIdx >= 0) m_Rooms[prevKeyRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::Room);
			if (prevDoorRoomIdx >= 0) m_Rooms[prevDoorRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::Room);

			int keyRoomIdx{};
			do
			{
				keyRoomIdx = rand() % m_Rooms.size();
			} while (m_Rooms[keyRoomIdx].GetRoomType() != DungeonRoom::DungeonRoomType::Room || keyRoomIdx == startIdx || keyRoomIdx == endIdx);

			prevKeyRoomIdx = keyRoomIdx;
			m_Rooms[keyRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);

			int doorRoomIdx{};
			do
			{
				doorRoomIdx = rand() % m_Rooms.size();
			} while (m_Rooms[doorRoomIdx].GetRoomType() != DungeonRoom::DungeonRoomType::Room || 
				keyRoomIdx == startIdx || 
				keyRoomIdx == endIdx ||
				m_Rooms[doorRoomIdx].GetConnections().size() == 1);

			prevDoorRoomIdx = doorRoomIdx;
			m_Rooms[doorRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);
		} while (!solver.Solve());

		keyRooms.push_back(prevKeyRoomIdx);
		lockedRooms.push_back(prevDoorRoomIdx);
	}

	for (int roomIdx : keyRooms)
	{
		m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);
	}
	for (int roomIdx : lockedRooms)
	{
		m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);
	}
}

void Dungeon::Update()
{
	m_Generator.Update(m_Rooms);
}

bool Dungeon::PickUpKeyInRoom(int roomIdx)
{
	if (m_Rooms[roomIdx].GetRoomType() != DungeonRoom::DungeonRoomType::KeyRoom) return false;

	m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::Room);

	return true;
}

bool Dungeon::UseKeyInRoom(int roomIdx)
{
	if (m_Rooms[roomIdx].GetRoomType() != DungeonRoom::DungeonRoomType::LockedRoom) return false;

	m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::Room);

	return true;
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

bool Dungeon::IsRoomLocked(int roomIdx) const
{
	return m_Rooms[roomIdx].GetRoomType() == DungeonRoom::DungeonRoomType::LockedRoom;
}

bool Dungeon::IsSolved() const
{
	for (const DungeonRoom& room : m_Rooms)
	{
		if (room.GetRoomType() == DungeonRoom::DungeonRoomType::KeyRoom) return false;
		if (room.GetRoomType() == DungeonRoom::DungeonRoomType::LockedRoom) return false;
	}
	return true;
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
