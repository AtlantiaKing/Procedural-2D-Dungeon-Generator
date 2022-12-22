#pragma once
#include "DungeonRoom.h"
#include <vector>

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//-----------------------------------------------------
// Dungeon Class									
//-----------------------------------------------------
class Dungeon final
{
public:
	Dungeon();				// Constructor
	~Dungeon() = default;	// Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Dungeon(const Dungeon& other)					= delete;
	Dungeon(Dungeon&& other) noexcept				= delete;
	Dungeon& operator=(const Dungeon& other)		= delete;
	Dungeon& operator=(Dungeon&& other)	noexcept	= delete;

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
	std::vector<DungeonRoom> m_Rooms{};

};


