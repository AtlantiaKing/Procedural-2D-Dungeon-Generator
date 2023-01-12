//---------------------------
// Includes
//---------------------------
#define _USE_MATH_DEFINES
#include "DungeonGenerator.h"
#include "Utils.h"
#include "Camera.h"
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
void DungeonGenerator::GenerateDungeon(std::vector<DungeonRoom>& rooms)
{
	// Apply the seed
	if (m_CurrentSeed < 0)
	{
		srand(static_cast<unsigned int>(time(NULL)));
	}
	else
	{
		srand(static_cast<unsigned int>(m_CurrentSeed));
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

		// Enable slow dungeon generation
		m_IsGenerating = true;
	}
	else
	{
		// Disable slow dungeon generation
		m_IsGenerating = false;

		// Create rooms of random sizes inside a circle
		CreateRoomsInCircle(rooms);

		// Seperate all the rooms so none of the rooms overlap
		while (!SeperateRooms(rooms));

		// Only keep the biggest rooms
		DiscardSmallRooms(rooms);

		// Only keep rooms that are at a decent room from other rooms
		DiscardBorderingRooms(rooms);

		// If all rooms are removed
		if (rooms.empty())
		{
			// Generate a new dungeon
			GenerateDungeon(rooms);
			return;
		}

		// Triangulate the dungeon
		m_Triangulation.Triangulate(rooms);

		// If no triangle is created
		if (m_Triangulation.GetSize() < 3)
		{
			// Generate a new dungeon
			GenerateDungeon(rooms);
		}

		// Create the minimum spanning tree from the triangulated dungeon
		CreateMinimumSpanningTree();

		// Create corridors between the dungeon rooms
		CreateCorridors(rooms);

		// Choose the start and the end of the dungeon
		ChooseBeginAndEndRoom(rooms);

		// Set the generation state to "done"
		m_CurrentGenerationState = GenerationCycleState::DONE;
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
			m_CurrentGenerationState = GenerationCycleState::DISCARD_SMALL_ROOMS;
		}
		break;
	}
	case GenerationCycleState::DISCARD_SMALL_ROOMS:
	{
		// Discard small rooms, if all rooms are above the size threshold, switch to the triangulation state
		if (DiscardSmallRooms(rooms, true))
		{
			// If all rooms are removed
			if (rooms.empty())
			{
				// Generate a new dungeon
				GenerateDungeon(rooms);
			}
			else
			{
				// If there are still rooms in the dungeon, switch to triangulation state
				m_CurrentGenerationState = GenerationCycleState::DISCARD_BORDERING_ROOMS;
			}
		}
		break;
	}
	case GenerationCycleState::DISCARD_BORDERING_ROOMS:
	{
		// Discard small rooms, if all rooms are above the size threshold, switch to the triangulation state
		if (DiscardBorderingRooms(rooms, true))
		{
			// If all rooms are removed
			if (rooms.empty())
			{
				// Generate a new dungeon
				GenerateDungeon(rooms);
			}
			else
			{
				// If there are still rooms in the dungeon, switch to triangulation state
				m_CurrentGenerationState = GenerationCycleState::TRIANGULATION;
				m_Triangulation.StartTriangulation();
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
			m_Triangulation.AddPoint(rooms[m_CurTriangulateRoom].GetPosition() + rooms[m_CurTriangulateRoom].GetSize() / 2, m_CurTriangulateRoom);

			// Increment the amount of rooms added to the triangulation
			++m_CurTriangulateRoom;
		}
		else
		{
			// Finish the triangulation algorithm
			m_Triangulation.FinishTriangulation();

			// If no triangle is created
			if (m_Triangulation.GetSize() < 3)
			{
				// Generate a new dungeon
				GenerateDungeon(rooms);
			}
			else
			{
				// Switch to the spanning tree algorithm state
				m_CurrentGenerationState = GenerationCycleState::SPANNING_TREE_ALGORITHM;
			}
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
	{
		// Create corridors between the dungeon rooms
		CreateCorridors(rooms);

		// Choose the start and the end of the dungeon
		ChooseBeginAndEndRoom(rooms);

		// Switch to the finished state
		m_CurrentGenerationState = GenerationCycleState::DONE;
		break;
	}
	}
}

void DungeonGenerator::DrawDebug() const
{
	// Render the deleted rooms
	for (const DungeonRoom& room : m_DebugRooms)
	{
		room.Draw(true);
	}

	switch (m_CurrentGenerationState)
	{
	case DungeonGenerator::GenerationCycleState::CIRCLE:
	{
		// Render the circle in which rooms are generated
		const Vector2& scaledCenter{ CAMERA->ScalePoint(m_Center) };
		const int scaledRadius{ CAMERA->ScaleSize(static_cast<int>(m_InitRadius * 2)) };
		GAME_ENGINE->DrawOval(scaledCenter.x, scaledCenter.y, scaledRadius, scaledRadius);
		break;
	}
	case DungeonGenerator::GenerationCycleState::TRIANGULATION:
	{
		// Render the triangulation
		m_Triangulation.Draw();
		break;
	}
	case DungeonGenerator::GenerationCycleState::SPANNING_TREE_ALGORITHM:
	case DungeonGenerator::GenerationCycleState::CORRIDORS:
	{
		// Render each edge of the minimum spanning tree
		GAME_ENGINE->SetColor(RGB(0, 0, 255));
		for (const Edge& edge : m_MinimumSpanningTree)
		{
			const Vector2& scaledP0{ CAMERA->ScalePoint(edge.p0.first) };
			const Vector2& scaledP1{ CAMERA->ScalePoint(edge.p1.first) };
			GAME_ENGINE->DrawLine(scaledP0.x, scaledP0.y, scaledP1.x, scaledP1.y);
		}
		break;
	}
	}
}

bool DungeonGenerator::IsDone() const
{
	return m_CurrentGenerationState == GenerationCycleState::DONE;
}

int DungeonGenerator::GetInitialRoomCount() const
{
	return m_InitRoomCount;
}

int DungeonGenerator::GetInitialRadius() const
{
	return m_InitRadius;
}

void DungeonGenerator::SetRoomSizeBounds(int minSize, int maxSize)
{
	m_RoomSizeBounds.x = minSize;
	m_RoomSizeBounds.y = maxSize;
}

void DungeonGenerator::CreateRoomsInCircle(std::vector<DungeonRoom>& rooms) const
{	
	// For every room
	for (int i{}; i < m_InitRoomCount; ++i)
	{
		CreateRoomInCircle(rooms);
	}
}

void DungeonGenerator::CreateRoomInCircle(std::vector<DungeonRoom>& rooms) const
{
	constexpr float pi{ static_cast<float>(M_PI) };

	// Calculate a random offset from the center
	const float r = m_InitRadius * sqrtf(RandomFloat(0.0f, 1.0f));

	// Calculate a random angle
	const float theta = RandomFloat(0.0f, 1.0f) * 2 * pi;

	// Calculate the position
	const Vector2 pos
	{
		static_cast<int>(r * cosf(theta)),
		static_cast<int>(r * sinf(theta))
	};

	// Calculate a random size
	const Vector2 size
	{
		RandomInt(m_RoomSizeBounds.x, m_RoomSizeBounds.y),
		RandomInt(m_RoomSizeBounds.x, m_RoomSizeBounds.y)
	};

	// The color that the room should be drawn in
	constexpr Color roomColor{ 255, 0 ,0 };

	// Create a room and add it to the container
	rooms.push_back(DungeonRoom{ m_Center + pos, size, roomColor });
}

bool DungeonGenerator::SeperateRooms(std::vector<DungeonRoom>& rooms) const
{
	// The maximum speed a room can have compared to another room
	constexpr int maxSpeed{ 3 };

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

			// Makes sure false gets returned, which will repeat the seperation
			isEveryRoomSeperated = false;

			// Calculate the direction between the rooms
			Vector2 curDirection{ room.GetPosition() + room.GetSize() / 2 - (otherRoom.GetPosition() + otherRoom.GetSize() / 2) };
			// Normalize the direction
			curDirection.ToDirection();

			// Set the direction to max speed
			curDirection *= maxSpeed;

			// Add the current direction to the total direction
			seperationDirection += curDirection;
		}

		// Move the room to the calculated seperation direction
		room.Move(seperationDirection);
	}

	// Return wether all rooms are not overlapping anymore or not
	return isEveryRoomSeperated;
}

bool DungeonGenerator::DiscardSmallRooms(std::vector<DungeonRoom>& rooms, bool debug)
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

			if (debug)
			{
				// Return false so the while loop continues
				return false;
			}
		}
	}

	// Return true so the while loop stops
	return true;
}

bool DungeonGenerator::DiscardBorderingRooms(std::vector<DungeonRoom>& rooms, bool debug)
{
	// The minimal amount of space between two rooms
	constexpr int minCorridorSize{ 10 };

	// Loop over all the rooms
	for (int i{ static_cast<int>(rooms.size()) - 1 }; i >= 0; --i)
	{
		// Get the current room
		const DungeonRoom& room{ rooms[i] };

		// Loop over all the rooms
		for (int j{ static_cast<int>(rooms.size()) - 1 }; j >= 0; --j)
		{
			if (i == j) continue;

			// Get the current room
			const DungeonRoom& other{ rooms[j] };

			// Distance between the rooms
			const Vector2 distance{ room.GetPosition() + room.GetSize() / 2 - (other.GetPosition() + other.GetSize() / 2) };
			const int roomDistX{ abs(distance.x) };
			const int roomDistY{ abs(distance.y) };

			// The amount of space needed between buildings to have the minimal corridor
			const int minCorridorSizeSpaceX{ room.GetSize().x / 2 + other.GetSize().x / 2 + minCorridorSize };
			const int minCorridorSizeSpaceY{ room.GetSize().y / 2 + other.GetSize().y / 2 + minCorridorSize };

			// Get which kind of corridor will be created
			const bool isCorridorFlat{ roomDistX > roomDistY };

			// If the rooms are too close to each other depending on which corridor will be created
			if (isCorridorFlat && roomDistX < minCorridorSizeSpaceX || !isCorridorFlat && roomDistY < minCorridorSizeSpaceY)
			{
				// Add the room to the debug room list, this will make sure the rooms are still drawn, but in a different color
				m_DebugRooms.push_back(room);

				// Remove the current room from the list of rooms
				rooms[i] = rooms[rooms.size() - 1];
				rooms.pop_back();

				if (debug)
				{
					// Return false so the while loop continues
					return false;
				}
				break;
			}
		}
	}

	// Return false so the while loop stops
	return true;
}

void DungeonGenerator::CreateMinimumSpanningTree()
{
	// Collection of trees
	std::vector<Tree> forest{};
	
	// All the edges of the triangulation, sorted by length
	std::set<Edge> edges{};
	m_Triangulation.CreateSetOfEdges(edges);

	// The amount of vertices/rooms
	const size_t nrVertices{ m_Triangulation.GetSize() };

	// As long as there are edges to check and the amount of edges in the MST is less then the amount of vertices - 1
	while (!edges.empty() && (forest.empty() || forest[0].edges.size() < nrVertices - 1))
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
			const TreeConnectionState connection{ tree.IsConnected(curEdge) };

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
				// If the edge is already connected to a tree, merge the two trees and remove the current tree
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
			forest.push_back(Tree{ { curEdge }, { curEdge.p0.second, curEdge.p1.second } });
		}
		else
		{
			// If the edge is connected to a tree, add the edge to the edges of the tree
			forest[connectedTreeIdx].edges.push_back(curEdge);
			forest[connectedTreeIdx].vertices.insert(curEdge.p0.second);
			forest[connectedTreeIdx].vertices.insert(curEdge.p1.second);
		}
	}

	// Move the main tree (the minimum spanning tree) to a member variable
	m_MinimumSpanningTree = std::move(forest[0].edges);
}

void DungeonGenerator::CreateCorridors(std::vector<DungeonRoom>& rooms) const
{
	// The minimum allowed size of the corridor
	constexpr int minSize{ 20 };

	// The color of corridors
	constexpr Color corridorColor{ 255, 127, 127 };

	// For every edge in the MST
	for (const Edge& edge : m_MinimumSpanningTree)
	{
		// Get the two rooms connected to this edge
		DungeonRoom& room0{ rooms[edge.p0.second] };
		DungeonRoom& room1{ rooms[edge.p1.second] };

		// Get the center of each room
		const Vector2 room0Pos{ room0.GetPosition() + room0.GetSize() / 2 };
		const Vector2 room1Pos{ room1.GetPosition() + room1.GetSize() / 2 };

		// If the x and y directions go from room 0 to 1 or not
		bool directionX01{};
		bool directionY01{};

		// The bottom left corner of the new corridor
		Vector2 bottomLeft{};

		// Set the x direction and the x component of the leftbottom corner
		if (room0Pos.x < room1Pos.x)
		{
			bottomLeft.x = room0Pos.x;
			directionX01 = true;
		}
		else
		{
			bottomLeft.x = room1Pos.x;
		}

		// Set the y direction and the y component of the leftbottom corner
		if (room0Pos.y < room1Pos.y)
		{
			bottomLeft.y = room0Pos.y;
			directionY01 = true;
		}
		else
		{
			bottomLeft.y = room1Pos.y;
		}

		// The size of the corridor
		Vector2 size{ abs(room0Pos.x - room1Pos.x), abs(room0Pos.y - room1Pos.y) };

		// If the size of the corridor is smaller then the minimum allowed size, scale and move the corridor
		if (size.x < minSize)
		{
			const int sizeGrow{ minSize - size.x };
			size.x = minSize;
			bottomLeft.x -= sizeGrow / 2;
		}
		if (size.y < minSize)
		{
			const int sizeGrow{ minSize - size.y };
			size.y = minSize;
			bottomLeft.y -= sizeGrow / 2;
		}

		// The displacements at the left, top, right and bottom
		int leftDisplacement{};
		int rightDisplacement{};
		int bottomDisplacement{};
		int topDisplacement{};

		// Is this room a vertical or horizontal corridor
		const bool isHorizontalCorridor{ size.x > size.y };

		// Depending on a horizontal or vertical corridor, calculate the displacement amounts
		if (isHorizontalCorridor)
		{
			if (directionX01)
			{
				leftDisplacement = (room0Pos + room0.GetSize() / 2).x - bottomLeft.x;
				rightDisplacement = bottomLeft.x + size.x - (room1Pos - room1.GetSize() / 2).x;
			}
			else
			{
				leftDisplacement = (room1Pos + room1.GetSize() / 2).x - bottomLeft.x;
				rightDisplacement = bottomLeft.x + size.x - (room0Pos - room1.GetSize() / 2).x;
			}
		}
		else
		{
			if (directionY01)
			{
				bottomDisplacement = (room0Pos + room0.GetSize() / 2).y - bottomLeft.y;
				topDisplacement = bottomLeft.y + size.y - (room1Pos - room1.GetSize() / 2).y;
			}
			else
			{
				bottomDisplacement = (room1Pos + room1.GetSize() / 2).y - bottomLeft.y;
				topDisplacement = bottomLeft.y + size.y - (room0Pos - room0.GetSize() / 2).y;
			}
		}

		// Move and scale the corridor according to the calculated displacements
		bottomLeft.x += leftDisplacement;
		size.x -= leftDisplacement;
		size.x -= rightDisplacement;
		bottomLeft.y += bottomDisplacement;
		size.y -= bottomDisplacement;
		size.y -= topDisplacement;

		// If the size of the corridor has become 0 or less, continue to the next corridor
		if (size.x <= 0 || size.y <= 0) continue;

		// Create a new room
		DungeonRoom newRoom
		{
			bottomLeft,
			size,
			corridorColor
		};

		// Add the neighbouring rooms to the connections of the corridor
		newRoom.AddConnection(edge.p0.second);
		newRoom.AddConnection(edge.p1.second);

		// Add the corridor to the list of rooms
		rooms.push_back(newRoom);

		// Add the new room to the connections of the neighbouring rooms
		room0.AddConnection(static_cast<int>(rooms.size() - 1));
		room1.AddConnection(static_cast<int>(rooms.size() - 1));
	}
}

void DungeonGenerator::ChooseBeginAndEndRoom(std::vector<DungeonRoom>& rooms) const
{
	// The index of the start room
	int startIdx{ -1 };
	
	// The current greatest distance
	int distanceFromStart{ 0 };

	// The index of the end room
	int endIdx{ -1 };

	// For each room
	for (int i{}; i < rooms.size(); ++i)
	{
		// The number of connections this room has
		int nrConnections{};

		// For each edge in the MST
		for (const Edge& edge : m_MinimumSpanningTree)
		{
			// If the edge contains this room, increment the number of connections
			if (edge.p0.second == i || edge.p1.second == i)
				++nrConnections;
		}

		// If this room is not a leaf room, continue to the next room
		if (nrConnections != 1) continue;

		if (startIdx == -1)
		{
			// If a start room has not been found, set the start room to the current room
			startIdx = i;
		}
		else
		{
			// If a start room has been found

			// Calculate the distance between the start and end room
			const int curDistanceFromStart{ rooms[i].GetPosition().DistanceSqr(rooms[startIdx].GetPosition()) };

			// If the calculated distance is greater then the current greatest distance
			if (curDistanceFromStart > distanceFromStart)
			{
				// Set the end room and its distance to the current room and distance
				distanceFromStart = curDistanceFromStart;
				endIdx = i;
			}
		}
	}

	// Set the properties of the start and end room
	rooms[startIdx].SetColor(Color{ 255, 215, 0 });	// Gold color
	rooms[startIdx].SetRoomType(DungeonRoom::DungeonRoomType::Start);
	rooms[endIdx].SetColor(Color{ 50, 50, 50 }); // Dark gray color
	rooms[endIdx].SetRoomType(DungeonRoom::DungeonRoomType::End);
}
