#include "DungeonSolver.h"
#include "Dungeon.h"

std::vector<int> DungeonSolver::m_ShortestPath{};

DungeonSolver::DungeonSolver(std::shared_ptr<Dungeon> dungeon)
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

	// Add the end room to the final path
	m_TotalPath.push_back(endRoom);

	// Has the solver solved the dungeon
	const bool hasSolvedDungeon{ m_CurRoom == endRoom };

	// If the solver has solved the dungeon and it should save the shortest route
	if (hasSolvedDungeon && saveShortestRoute)
	{
		SaveShortestRoute();
	}

	// Return whether if the dungeon has been solved
	return hasSolvedDungeon && (!m_NeedAllKeys || m_pDungeon->IsSolved());
}

bool DungeonSolver::HasDiscovered(int roomIdx) const
{
	for (int discoveredRoomIdx : m_DiscoveredRooms)
	{
		if (roomIdx == discoveredRoomIdx) return true;
	}
	return false;
}

void DungeonSolver::SaveShortestRoute() const
{
	// Clear any previous path
	m_ShortestPath.clear();

	// For each room in the total path
	for (int roomIdx : m_TotalPath)
	{
		// Check if the current room is already been visited on the shortest path
		bool alreadyVisited{};
		for (int visitedIdx : m_ShortestPath)
		{
			if (visitedIdx != roomIdx) continue;

			alreadyVisited = true;
			break;
		}

		if (!alreadyVisited)
		{
			// If this current room is not yet visited on the shorest path, add it to the path
			m_ShortestPath.push_back(roomIdx);
		}
		else
		{
			// If this current room is already been visited, pop the shortest path until it reaches the current room
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

	// If there is a forced path
	if (!m_ForcedPath.empty())
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
	
	if (m_pDungeon->IsRoomLocked(m_CurRoom)) // If the room is locked
	{
		// If the solver has no keys
		if (m_NrKeys == 0)
		{
			// Return to the previous room
			m_CurRoom = m_PreviousRooms.top();
			m_PreviousRooms.pop();

			return true;
		}

		// Try to use a key, if succeeded, decrement number of keys
		if (m_pDungeon->UseKeyInRoom(m_CurRoom)) --m_NrKeys;
	}
	else if (m_pDungeon->PickUpKeyInRoom(m_CurRoom)) // If the solver picked up a key in this room
	{
		// Increment the number of keys
		++m_NrKeys;

		// Get the first door on the shortest path
		int lockedRoomIdx{ -1 };
		for (int i{}; i < static_cast<int>(m_ShortestPath.size()); ++i)
		{
			if (m_pDungeon->IsRoomLocked(m_ShortestPath[i]))
			{
				lockedRoomIdx = m_ShortestPath[i];
				break;
			}
		}

		// If no locked room is found, return
		if (lockedRoomIdx < 0) return false;

		// Has the solver seen this locked room
		bool hasSeenDoor{};
		// The index of the locked room in the total path
		int roomIdxInFinalPath{};

		// For each room on the total path
		for (int i{ static_cast<int>(m_TotalPath.size()) - 1 }; i >= 0; --i)
		{
			// If the room in total path is not the locked room, continue to the next room
			if (m_TotalPath[i] != lockedRoomIdx) continue;

			// Store the index of this room
			roomIdxInFinalPath = i;
			hasSeenDoor = true;
			break;
		}

		// If the solver has seen this door
		if (hasSeenDoor)
		{
			// Fill the forced path with everything in the total path until the door
			for (int i{ roomIdxInFinalPath }; i < m_TotalPath.size() - 1; ++i)
			{
				m_ForcedPath.push_back(m_TotalPath[i]);
			}

			return true;
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
		if (m_PreviousRooms.empty()) return false;

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
