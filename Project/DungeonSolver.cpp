#include "DungeonSolver.h"
#include "Dungeon.h"

DungeonSolver::DungeonSolver(Dungeon* dungeon)
	: m_pDungeon { dungeon }
{
}

bool DungeonSolver::Solve()
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

	// Return whether if the dungeon has been solved
	return m_CurRoom == endRoom && m_pDungeon->IsSolved();
}

bool DungeonSolver::HasDiscovered(int roomIdx) const
{
	for (int discoveredRoomIdx : m_DiscoveredRooms)
	{
		if (roomIdx == discoveredRoomIdx) return true;
	}
	return false;
}

bool DungeonSolver::SolveStep()
{
	// Save the current room in the total path container
	m_TotalPath.push_back(m_CurRoom);

	// If this room has not yet been discovered, add it to the discovered rooms container
	if (!HasDiscovered(m_CurRoom)) m_DiscoveredRooms.push_back(m_CurRoom);

	if (m_pDungeon->PickUpKeyInRoom(m_CurRoom))
	{
		int doorRoomIdx{ -1 };
		for (int i{ static_cast<int>(m_TotalPath.size() - 1) }; i >= 0; --i)
		{
			if (m_pDungeon->IsRoomLocked(m_TotalPath[i]))
			{
				doorRoomIdx = i;
				break;
			}
		}

		if (doorRoomIdx >= 0)
		{
			for (int i{ static_cast<int>(m_TotalPath.size() - 1) }; i >= doorRoomIdx; --i)
			{
				const auto removeIt
				{ 
					std::remove_if(
					m_DiscoveredRooms.begin(),
					m_DiscoveredRooms.end(),
					[&](int room)
					{
						return room == m_TotalPath[i];
					}) 
				};

				if (removeIt != m_DiscoveredRooms.end())
					m_DiscoveredRooms.erase(removeIt);
			}
		}
		++m_NrKeys;
	}

	// Get all the connections out of this room
	const std::vector<int>& connections{ m_pDungeon->GetRoomConnectionsFromIndex(m_CurRoom) };

	// Whether a new room has been found
	bool foundNewRoom{};
	
	// Loop over all connections
	for (int connection : connections)
	{
		// If this connection has already been discovered, continue to the next room
		if (HasDiscovered(connection)) continue;

		if (m_pDungeon->IsRoomLocked(m_CurRoom))
		{
			if (m_NrKeys == 0) continue;

			if (m_pDungeon->UseKeyInRoom(m_CurRoom)) --m_NrKeys;
		}

		foundNewRoom = true;

		// Save the current room in the previous rooms container
		m_PreviousRooms.push(m_CurRoom);

		// Move to the new room
		m_CurRoom = connection;

		break;
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

	return true;
}
