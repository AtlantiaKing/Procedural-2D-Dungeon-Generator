#pragma once
#include <vector>
#include "DungeonRoom.h"

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//-----------------------------------------------------
// DungeonGenerator Class									
//-----------------------------------------------------
class DungeonGenerator final
{
public:
	DungeonGenerator() = default;				// Constructor
	~DungeonGenerator() = default;				// Destructor

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	static void GenerateDungeon(std::vector<DungeonRoom>& rooms);

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------


};
