//---------------------------
// Includes
//---------------------------
#define _USE_MATH_DEFINES
#include "DungeonGenerator.h"
#include "Utils.h"

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
	// Clear the rooms container
	rooms.clear();

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
		// Seperate rooms, if all rooms are not overlapping anymore, switch to the triangulation state
		if (SeperateRooms(rooms))
		{
			m_CurrentGenerationState = GenerationCycleState::TRIANGULATION;
		}
		break;
	}
	case GenerationCycleState::TRIANGULATION:
		break;
	case GenerationCycleState::SPANNING_TREE_ALGORITHM:
		break;
	case GenerationCycleState::CORRIDORS:
		break;
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
