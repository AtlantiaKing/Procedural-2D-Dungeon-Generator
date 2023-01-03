//---------------------------
// Includes
//---------------------------
#define _USE_MATH_DEFINES
#include "DungeonGenerator.h"
#include "Utils.h"
#include <set>

//---------------------------
// Constructor & Destructor
//---------------------------
DungeonGenerator::DungeonGenerator()
	: m_Center{ GAME_ENGINE->GetWidth() / 2, GAME_ENGINE->GetHeight() / 2 }
{
}

//---------------------------
// Member functions
//---------------------------
void DungeonGenerator::GenerateDungeon(int seed, std::vector<DungeonRoom>& rooms)
{
	// Save the seed
	m_CurrentSeed = seed;

	// Apply the seed
	if (seed < 0)
	{
		srand(time(NULL));
	}
	else
	{
		srand(static_cast<unsigned int>(seed));
	}

	// Clear the rooms container
	m_DebugRooms.clear();
	rooms.clear();

	// Clear the triangulation
	m_Triangulation.Clear();
	m_CurTriangulateRoom = 0;

	// Clear the minimum spanning tree
	m_MinimumSpanningTree.clear();

	if (m_IsSlowlyGenerating)
	{
		// Reset the generation to circle generation
		m_CurrentGenerationState = GenerationCycleState::CIRCLE;

		// Enable the dungeon generation
		m_IsGenerating = true;
	}
	else
	{
		// Disable slow generation
		m_IsGenerating = false;

		// Create rooms of random sizes inside a circle
		CreateRoomsInCircle(rooms);

		// Seperate all the rooms so none of the rooms overlap
		while (!SeperateRooms(rooms));

		// Only keep the biggest rooms
		while(!DiscardSmallRooms(rooms));

		// If all rooms are removed
		if (rooms.size() == 0)
		{
			// Generate a new dungeon
			GenerateDungeon(seed, rooms);
		}

		// Triangulate the dungeon
		m_Triangulation.Triangulate(GAME_ENGINE->GetWidth(), rooms);

		// Create the minimum spanning tree from the triangulated dungeon
		CreateMinimumSpanningTree();
	}
}

void DungeonGenerator::Update(std::vector<DungeonRoom>& rooms)
{
	// If the dungeon is not slowly generating, do nothing
	if (!m_IsSlowlyGenerating) return;

	// If the dungeon is already generated, do nothing
	if (!m_IsGenerating) return;

	// Switch between every generation cycle state
	switch (m_CurrentGenerationState)
	{
	case GenerationCycleState::CIRCLE:
	{
		// Generate a new room
		CreateRoomInCircle(rooms);

		// If the max amount of rooms is reached, switch to the seperation state
		if (rooms.size() == m_InitRoomCount)
		{
			m_CurrentGenerationState = GenerationCycleState::SEPERATION;
		}
		break;
	}
	case GenerationCycleState::SEPERATION:
	{
		// Seperate rooms, if all rooms are not overlapping anymore, switch to the discard rooms state
		if (SeperateRooms(rooms))
		{
			m_CurrentGenerationState = GenerationCycleState::DISCARD_ROOMS;
		}
		break;
	}
	case GenerationCycleState::DISCARD_ROOMS:
	{
		// Discard small rooms, if all rooms are above the size threshold, switch to the triangulation state
		if (DiscardSmallRooms(rooms))
		{
			// If all rooms are removed
			if (rooms.size() == 0)
			{
				// Generate a new dungeon
				GenerateDungeon(m_CurrentSeed, rooms);
			}
			else
			{
				// If there are still rooms in the dungeon, switch to triangulation state
				m_CurrentGenerationState = GenerationCycleState::TRIANGULATION;
				m_Triangulation.StartTriangulation(GAME_ENGINE->GetWidth());
			}
		}
		break;
	}
	case GenerationCycleState::TRIANGULATION:
	{
		// If not every room has been added to the triangulation
		if (m_CurTriangulateRoom < rooms.size())
		{
			// Add the center of the room to the triangulation
			m_Triangulation.AddPoint(rooms[m_CurTriangulateRoom].GetPosition() + rooms[m_CurTriangulateRoom].GetSize() / 2);

			// Increment the amount of rooms added to the triangulation
			++m_CurTriangulateRoom;
		}
		else
		{
			// Finish the triangulation algorithm
			m_Triangulation.FinishTriangulation();

			// Switch to the spanning tree algorithm state
			m_CurrentGenerationState = GenerationCycleState::SPANNING_TREE_ALGORITHM;
		}
		break;
	}
	case GenerationCycleState::SPANNING_TREE_ALGORITHM:
	{
		// Create the minimum spanning tree from the triangulated dungeon
		CreateMinimumSpanningTree();

		// Switch to the corridor creation state
		m_CurrentGenerationState = GenerationCycleState::CORRIDORS;
		break;
	}
	case GenerationCycleState::CORRIDORS:
		break;
	}
}

void DungeonGenerator::RenderDebug() const
{
	// Render the deleted rooms
	for (const DungeonRoom& room : m_DebugRooms)
	{
		room.Draw(true);
	}

	if (m_MinimumSpanningTree.size() > 0)
	{
		GAME_ENGINE->SetColor(RGB(0, 0, 255));
		for (const Edge& edge : m_MinimumSpanningTree)
		{
			GAME_ENGINE->DrawLine(edge.p0.x, edge.p0.y, edge.p1.x, edge.p1.y);
		}
	}
	else
	{
		// Render the triangulation
		m_Triangulation.Draw();
	}
}

void DungeonGenerator::SetRoomSizeBounds(int minSize, int maxSize)
{
	m_RoomSizeBounds.x = minSize;
	m_RoomSizeBounds.y = maxSize;
}

void DungeonGenerator::CreateRoomsInCircle(std::vector<DungeonRoom>& rooms)
{	
	// For every room
	for (int i{}; i < m_InitRoomCount; ++i)
	{
		CreateRoomInCircle(rooms);
	}
}

void DungeonGenerator::CreateRoomInCircle(std::vector<DungeonRoom>& rooms)
{
	const float pi{ static_cast<float>(M_PI) };

	// Calculate a random offset from the center
	float r = m_InitRadius * sqrtf(RandomFloat(0.0f, 1.0f));

	// Calculate a random angle
	float theta = RandomFloat(0.0f, 1.0f) * 2 * pi;

	// Calculate the position
	Vector2 pos
	{
		static_cast<int>(r * cosf(theta)),
		static_cast<int>(r * sinf(theta))
	};

	// Calculate a random size
	Vector2 size
	{
		RandomInt(m_RoomSizeBounds.x, m_RoomSizeBounds.y),
		RandomInt(m_RoomSizeBounds.x, m_RoomSizeBounds.y)
	};

	// Create a room and add it to the container
	rooms.push_back(DungeonRoom{ m_Center + pos, size, Color{ 255, 0 ,0 } });
}

bool DungeonGenerator::SeperateRooms(std::vector<DungeonRoom>& rooms)
{
	// Wether all rooms are not overlapping anymore
	bool isEveryRoomSeperated{ true };

	// For each room
	for (DungeonRoom& room : rooms)
	{
		// The total direction to move in
		Vector2 seperationDirection{};

		// For every other room
		for (DungeonRoom& otherRoom : rooms)
		{
			// If the other room is the same as the current room, continue to the next room
			if (&room == &otherRoom) continue;
			// If the rooms are not overlapping, continue to the next room
			if (!room.IsOverlapping(otherRoom)) continue;
			// If the distance between the rooms is more then the checking radius, continue to the next room
			if (room.GetPosition().DistanceSqr(otherRoom.GetPosition()) > m_InitRadius * m_InitRadius) continue;

			// Makes sure false gets returned, which will repeat the seperation
			isEveryRoomSeperated = false;

			// Calculate the direction between the rooms
			Vector2 curDirection{ room.GetPosition() - otherRoom.GetPosition() };
			// Normalize the direction and retrieve the distance
			const float distance{ curDirection.ToDirection() };

			// Add the current direction to the total direction, depending on the distance of the other room
			seperationDirection += curDirection * static_cast<int>(m_InitRadius * (1.0f - distance / m_InitRadius));
		}

		// Normalize the total direction and move in that direction
		seperationDirection.ToDirection();
		room.Move(seperationDirection);
	}

	// Return wether all rooms are not overlapping anymore or not
	return isEveryRoomSeperated;
}

bool DungeonGenerator::DiscardSmallRooms(std::vector<DungeonRoom>& rooms)
{
	// Loop over all the rooms
	for(int i{ static_cast<int>(rooms.size()) - 1 }; i >= 0; --i)
	{
		// Get the current room
		const DungeonRoom& room{ rooms[i] };

		// If one of the size parameters is smaller then the threshold
		if (room.GetSize().x < m_RoomSizeThreshold || room.GetSize().y < m_RoomSizeThreshold)
		{
			// Add the room to the debug room list, this will make sure the rooms are still drawn, but in a different color
			m_DebugRooms.push_back(room);

			// Remove the current room from the list of rooms
			rooms[i] = rooms[rooms.size() - 1];
			rooms.pop_back();

			return false;
		}
	}

	return true;
}

void DungeonGenerator::CreateMinimumSpanningTree()
{
	// Collection on trees
	std::vector<Tree> forest{};
	
	// All the edges of the triangulation, sorted by length
	std::set<Edge> edges{};
	m_Triangulation.CreateSetOfEdges(edges);

	// The amount of vertices/rooms
	const size_t nrVertices{ m_Triangulation.GetSize() };

	// As long as there are edges to be tested or as long as not every room has been added to the main tree
	while (edges.size() > 0 && (forest.size() == 0 || forest[0].edges.size() < nrVertices))
	{
		// Get the shortest edge and remove it from the set
		Edge curEdge{ *edges.begin() };
		edges.erase(curEdge);

		bool createsLoop{};
		
		// The index of the tree it connects with
		int connectedTreeIdx{ -1 };

		// For each tree
		for (size_t i{}; i < forest.size(); ++i)
		{
			// Get a reference to the current tree
			const Tree& tree{ forest[i] };

			// Check whether the current edge connected with the tree
			TreeConnectionState connection{ tree.IsConnected(curEdge) };

			// If the edge makes the tree loop, continue to the next edge
			if (connection == TreeConnectionState::Loop)
			{
				createsLoop = true;
				break;
			}

			// If the edge is not connected to this tree, continue to the next tree
			if (connection != TreeConnectionState::Connected) continue;

			if (connectedTreeIdx == -1)
			{
				// If the edge is not connected to any tree yet, save the index of the current tree
				connectedTreeIdx = static_cast<int>(i);
			}
			else
			{
				// If the edge is already conntected to a tree, merge the two trees and remove the current tree
				forest[connectedTreeIdx].Merge(tree);
				forest[i] = forest[forest.size() - 1];
				forest.pop_back();
				break;
			}
		}

		// If the edge makes the tree loop, continue to the next edge
		if (createsLoop) continue;

		if (connectedTreeIdx == -1)
		{
			// If the edge is not connected to any tree, create a new tree
			forest.push_back(Tree{ { curEdge } });
		}
		else
		{
			// If the edge is connected to a tree, add the edge to the edges of the tree
			forest[connectedTreeIdx].edges.push_back(curEdge);
		}
	}

	// Move the main tree (the minimum spanning tree) to a variable
	m_MinimumSpanningTree = std::move(forest[0].edges);
}
