#pragma once
#include "DataTypes.h"

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//-----------------------------------------------------
// DungeonRoom Class									
//-----------------------------------------------------
class DungeonRoom final
{
public:
	DungeonRoom() = default;									// Constructors
	DungeonRoom(const Vector2 position, const Vector2 size, const Color color);
	~DungeonRoom()	= default;									// Destructor

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Draw() const;

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Vector2 m_Position{};
	Vector2 m_Size{};
	Color m_Color{};
};
