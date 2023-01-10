#include "DungeonSolver.h"
#include "Dungeon.h"

std::vector<int> DungeonSolver::m_ShortestPath{};

DungeonSolver::DungeonSolver(Dungeon* dungeon)
	: m_pDungeon { dungeon }
{
}

bool DungeonSolver::Solve(bool saveShortestRoute)
{
	// Reset the previous rooms and discovered rooms
	m_PreviousRooms = std::stack<int>();
	m_DiscoveredRooms.clear();
	m_TotalPath.clear();
	m_NrKeys = 0;

	// Set the current room to the start room
	m_CurRoom = m_pDungeon->GetStartRoom();

	// Get the end room
	const int endRoom{ m_pDungeon->GetEndRoom() };

	// While the solver has not reached the end room
	while (m_CurRoom != endRoom)
	{
		// Solve a step of the dungeon
		// If this return false, an error has occured which cancels the while loop
		if (!SolveStep()) break;
	}

	m_TotalPath.push_back(endRoom);

	if (m_CurRoom == endRoom && saveShortestRoute)
	{
		SaveShortestRoute();
	}

	// Return whether if the dungeon has been solved
	return m_CurRoom == endRoom && (!m_NeedAllKeys || m_pDungeon->IsSolved());
}

bool DungeonSolver::HasDiscovered(int roomIdx) const
{
	for (int discoveredRoomIdx : m_DiscoveredRooms)
	{
		if (roomIdx == discoveredRoomIdx) return true;
	}
	return false;
}

void DungeonSolver::SaveShortestRoute()
{
	m_ShortestPath.clear();

	for (int roomIdx : m_TotalPath)
	{
		bool alreadyVisited{};
		for (int visitedIdx : m_ShortestPath)
		{
			if (visitedIdx != roomIdx) continue;

			alreadyVisited = true;
			break;
		}

		if (!alreadyVisited)
		{
			m_ShortestPath.push_back(roomIdx);
		}
		else
		{
			while (*(m_ShortestPath.end() - 1) != roomIdx)
			{
				m_ShortestPath.pop_back();
			}
		}
	}
}

bool DungeonSolver::SolveStep()
{
	// Save the current room in the total path container
	m_TotalPath.push_back(m_CurRoom);

	if (m_ForcedPath.size() > 0)
	{
		// Return to the next room on the forced path
		m_CurRoom = *(m_ForcedPath.end() - 1);
		m_ForcedPath.pop_back();

		// Save the current room in the previous rooms container
		m_PreviousRooms.push(m_CurRoom);

		return true;
	}

	// If this room has not yet been discovered, add it to the discovered rooms container
	if (!HasDiscovered(m_CurRoom)) m_DiscoveredRooms.push_back(m_CurRoom);
	
	if (m_pDungeon->IsRoomLocked(m_CurRoom))
	{
		if (m_NrKeys == 0)
		{
			// Return to the previous room
			m_CurRoom = m_PreviousRooms.top();
			m_PreviousRooms.pop();

			return true;
		}

		if (m_pDungeon->UseKeyInRoom(m_CurRoom)) --m_NrKeys;
	}
	else if (m_pDungeon->PickUpKeyInRoom(m_CurRoom))
	{
		++m_NrKeys;

		int doorRoomIdx{ -1 };
		for (int i{}; i < static_cast<int>(m_ShortestPath.size()); ++i)
		{
			if (m_pDungeon->IsRoomLocked(m_ShortestPath[i]))
			{
				doorRoomIdx = m_ShortestPath[i];
				break;
			}
		}

		if (doorRoomIdx >= 0)
		{
			bool hasSeenDoor{};
			int roomIdxInFinalPath{};
			for (int i{ static_cast<int>(m_TotalPath.size()) - 1 }; i >= 0; --i)
			{
				if (m_TotalPath[i] != doorRoomIdx) continue;

				roomIdxInFinalPath = i;
				hasSeenDoor = true;
				break;
			}

			if (hasSeenDoor)
			{
				for (int i{ roomIdxInFinalPath }; i < m_TotalPath.size() - 1; ++i)
				{
					m_ForcedPath.push_back(m_TotalPath[i]);
				}

				return true;
			}
		}
	}

	// Get all the connections out of this room
	const std::vector<int>& connections{ m_pDungeon->GetRoomConnectionsFromIndex(m_CurRoom) };

	// Whether a new room has been found
	bool foundNewRoom{};
	int nextRoom{};
	
	// Loop over all connections
	for (int connection : connections)
	{
		// If this connection has already been discovered, continue to the next room
		if (HasDiscovered(connection)) continue;

		foundNewRoom = true;

		// Save the new room
		nextRoom = connection;


		// If the next room is on the shortest path, take this route
		bool isOnShortestPath{};
		for (int shortestPathRoomIdx : m_ShortestPath)
		{
			if (nextRoom != shortestPathRoomIdx) continue;

			isOnShortestPath = true;
			break;
		}
		if (isOnShortestPath) break;
	}

	// If no connection has not yet been discovered
	if (!foundNewRoom)
	{
		// If there are no more previous rooms, stop solving this dungeon
		if (m_PreviousRooms.size() == 0) return false;

		// Return to the previous room
		m_CurRoom = m_PreviousRooms.top();
		m_PreviousRooms.pop();
	}
	else
	{
		// Save the current room in the previous rooms container
		m_PreviousRooms.push(m_CurRoom);

		// Move to the new room
		m_CurRoom = nextRoom;
	}

	return true;
}
