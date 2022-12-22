//---------------------------
// Includes
//---------------------------
#include "DungeonRoom.h"
#include "GameEngine.h"
#include "GameDefines.h"

DungeonRoom::DungeonRoom(const Vector2 position, const Vector2 size, const Color color)
	: m_Position{ position }
	, m_Size { size }
	, m_Color{ color }
{
}

//---------------------------
// Member functions
//---------------------------
void DungeonRoom::Draw() const
{
	GAME_ENGINE->SetColor(m_Color.GetColor());
	GAME_ENGINE->FillRect(m_Position.x, m_Position.y, m_Size.x, m_Size.y);
}
