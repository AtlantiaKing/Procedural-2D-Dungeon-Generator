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
	// Generate the rooms of the dungeon
	m_Generator.GenerateDungeon(m_Rooms);

	// Reset keys
	m_HasAddedKeys = false;
}

void Dungeon::Update()
{
	// Update the generator
	m_Generator.Update(m_Rooms);

	// If the generator is done and no keys have been generated, generate keys and locked rooms
	if (m_Generator.IsDone() && !m_HasAddedKeys)
	{
		m_HasAddedKeys = true;

		GenerateKeysAndLockedRooms();
	}
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

void Dungeon::SetNeedAllKeys(bool needAllKeys)
{
	m_NeedAllKeys = needAllKeys;
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

Vector2 Dungeon::GetRoomPositionFromIndex(int roomIdx) const
{
	return m_Rooms[roomIdx].GetPosition() + m_Rooms[roomIdx].GetSize() / 2;
}

const std::vector<int>& Dungeon::GetRoomConnectionsFromIndex(int roomIdx) const
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
	// Draw every room
	for (const DungeonRoom& room : m_Rooms)
	{
		room.Draw();
	}

	// If generation is still busy, draw debug
	if (!m_Generator.IsDone())
	{
		m_Generator.DrawDebug();
	}
}

void Dungeon::GenerateKeysAndLockedRooms()
{
	// Create a solver for the dungeon
	DungeonSolver solver{ shared_from_this() };
	solver.SetNeedAllKeys(m_NeedAllKeys);

	// Solve the empty dungeon, this calculates the shortest path to complete the dungeon without keys and doors
	solver.Solve(true);

	// Find the start and end room
	const int startIdx{ GetStartRoom() };
	const int endIdx{ GetEndRoom() };

	// Rooms that have keys and doors
	std::vector<int> keyRooms{};
	std::vector<int> lockedRooms{};

	// Get the number of leaf rooms (only 1 connection) in the dungeon
	int nrLeafRooms{};
	for (const DungeonRoom& room : m_Rooms)
	{
		if (room.GetConnections().size() == 1) ++nrLeafRooms;
	}

	// Remove the start and end rooms
	nrLeafRooms -= 2;

	// Keep a counter so there won't be an infinite while loop
	int tries{};
	constexpr int maxTries{ 100 };

	// For every key
	for (int i{}; i < m_NrKeys; ++i)
	{
		// The indices of the rooms that will be added to the key rooms container and the locked rooms container
		int keyRoomIdx{ -1 };
		int doorRoomIdx{ -1 };

		// Spawn a key and a locked room, while the dungeon cannot be solved
		do
		{
			++tries;

			// Reset the previous key and locked room if there were any
			if (keyRoomIdx >= 0) m_Rooms[keyRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::Room);
			if (doorRoomIdx >= 0) m_Rooms[doorRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::Room);

			// If the max tries have been hit, stop the loop
			if (tries > maxTries) break;

			// Reset the key and the lock in the already found key rooms and locked rooms
			for (int roomIdx : keyRooms)
			{
				m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);
			}
			for (int roomIdx : lockedRooms)
			{
				m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);
			}

			// Find a room that is not the start or the end, and the room is a leaf room if there are still leaf rooms available
			int curKeyRoomIdx{};
			do
			{
				curKeyRoomIdx = rand() % m_Rooms.size();
			} while (m_Rooms[curKeyRoomIdx].GetRoomType() != DungeonRoom::DungeonRoomType::Room ||
				curKeyRoomIdx == startIdx ||
				curKeyRoomIdx == endIdx ||
				(m_NeedAllKeys && tries < maxTries / 2 && keyRooms.size() < nrLeafRooms && m_Rooms[curKeyRoomIdx].GetConnections().size() > 1));

			// Save the current key room index and set the room type to a KeyRoom
			keyRoomIdx = curKeyRoomIdx;
			m_Rooms[curKeyRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);

			// Find a room that is not the start or the end, and the room is not a leaf room
			int curDoorRoomIdx{};
			do
			{
				curDoorRoomIdx = rand() % m_Rooms.size();
			} while (m_Rooms[curDoorRoomIdx].GetRoomType() != DungeonRoom::DungeonRoomType::Room ||
				curDoorRoomIdx == startIdx ||
				curDoorRoomIdx == endIdx ||
				m_Rooms[curDoorRoomIdx].GetConnections().size() == 1);

			// Save the current locked room index and set the room type to a LockedRoom
			doorRoomIdx = curDoorRoomIdx;
			m_Rooms[curDoorRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);

			// Can the current dungeon be solved
			bool solveable{ solver.Solve() };

			// If the dungeon can not be solved, swap the key and locked room and try again
			if (!solveable)
			{
				if (m_Rooms[curKeyRoomIdx].GetConnections().size() > 1)
				{
					curKeyRoomIdx = curDoorRoomIdx;
					curDoorRoomIdx = keyRoomIdx;
				}
				keyRoomIdx = curKeyRoomIdx;
				doorRoomIdx = curDoorRoomIdx;
				m_Rooms[curKeyRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);
				m_Rooms[curDoorRoomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);

				for (int roomIdx : keyRooms)
				{
					m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);
				}
				for (int roomIdx : lockedRooms)
				{
					m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);
				}
			}

		} while (!solver.Solve());

		// If the max tries have been hit, stop the loop
		if (tries > maxTries) break;

		// Add the found key and locked room to the list of key and locked rooms
		keyRooms.push_back(keyRoomIdx);
		lockedRooms.push_back(doorRoomIdx);
	}

	// Respawn the locks and keys
	for (int roomIdx : keyRooms)
	{
		m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::KeyRoom);
	}
	for (int roomIdx : lockedRooms)
	{
		m_Rooms[roomIdx].SetRoomType(DungeonRoom::DungeonRoomType::LockedRoom);
	}
}
