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
	CreateRoomsInCircle(rooms);
}

void DungeonGenerator::SetRoomSizeBounds(int minSize, int maxSize)
{
	m_RoomSizeBounds.x = minSize;
	m_RoomSizeBounds.y = maxSize;
}

void DungeonGenerator::CreateRoomsInCircle(std::vector<DungeonRoom>& rooms)
{
	const float pi{ static_cast<float>(M_PI) };
	
	for (int i{}; i < m_InitRoomCount; ++i)
	{
		float r = m_InitRadius * sqrtf(RandomFloat(0.0f, 1.0f));
		float theta = RandomFloat(0.0f, 1.0f) * 2 * pi;

		Vector2 pos
		{
			static_cast<int>(r * cosf(theta)), 
			static_cast<int>(r * sinf(theta))
		};
		Vector2 size
		{ 
			RandomInt(m_RoomSizeBounds.x, m_RoomSizeBounds.y),
			RandomInt(m_RoomSizeBounds.x, m_RoomSizeBounds.y)
		};

		rooms.push_back(DungeonRoom{ m_Center + pos, size, Color{ 255, 0 ,0 } });
	}
}
