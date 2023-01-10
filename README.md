

# Procedural 2D Dungeon Generator
## Introduction
#### The Project
This project is a 2D Dungeon Generator that can be used when designing dungeons for a game.  
It also includes an UI which has options such as size, amount of keys, etc.  

The project has been created in a c++ Win32 engine developed by a teacher at DAE of the university of Howest.  
The main techniques I used are seperation steering behavior, Delaunay triangulation and the minimum spanning tree algorithm.  
It also uses a simple dungeon solver AI to test if certain key and door placements don't make the dungeon impossible.

#### Motivation
I have always been fascinated by world generation. When I found an article about a dungeon generator using some techniques that I had seen in class, I was instantly intruiged.  
The process of generating the dungeon is inspired by the developer(s) of the game TinyKeep. My main goal in this project was to recreate the generation technique and add customizability to it. So it can be used as a tool when designing games.

#### Final result
![finalproduct](https://user-images.githubusercontent.com/35343159/211341835-f69ea012-e02a-41da-95ec-3774dd154981.gif)

## Table of content
- [Dungeon Layout Generation](#dungeon-layout-generation)
  * [Generate rooms](#step-1-generate-rooms)
  * [Seperation](#step-2-seperation)
  * [Discarding unusable rooms](#step-3-discarding-unusable-rooms)
  * [Triangulation](#step-4-triangulation)
  * [Minimum spanning tree](#step-5-minimum-spanning-tree)
  * [Generate corridors](#step-6-generate-corridors)
- [Sources](#sources)

## Dungeon Layout Generation
The dungeon layout generation (everything involving the placement of rooms) consists of 6 stages:
 1. Generate rooms
 2. Seperation
 3. Discard unusable rooms
 4. Triangulation
 5. Minimum spanning tree algorithm
 6. Generate corridors

![layoutgeneration](https://user-images.githubusercontent.com/35343159/211344324-ebe0604a-ff84-4253-84a1-621c1ba3b81f.gif)

In the next few sub-chapters I will explain each step of the layout generation progress in dept.

### Step 1: Generate rooms
During the first step a bunch of rooms are being created within a circle. The position of the room is calculated using a distance from center and a random angle.

Distance from center: $distance = radius * \sqrt{random(0.0, 1.0)}$  
Angle: $random(0.0, 1.0) * 2\pi$  
Position: $(distance * \cos{(angle)}, distance * \sin{(angle)})$  
The size of a room is a random number between two values.  

The radius of the circle and the amount of rooms that are created, can be set in the UI.

![generatingrooms](https://user-images.githubusercontent.com/35343159/211347234-240896bd-e8bd-4ac6-8119-1c93cae70ea7.gif)


### Step 2: Seperation
The second step is a seperation steering behavior on all the rooms. Each room seperates from all overlapping rooms.
For each overlapping room, a vector is calculated between the positions of the rooms. This vector is then converted to a direction vector that has 0, 1 or -1 as their components x and y.  
Because I use integers, not every direction vector will be normalized, which was not my goal. E.g. the vector (1,1)

    float length{ sqrtf(static_cast<float>(x * x + y * y)) };
	float xRanged{ static_cast<float>(x) / length };
	float yRanged{ static_cast<float>(y) / length };

	x = static_cast<int>(abs(xRanged) > 0.5f);
	if (xRanged < 0)
	{
		x *= -1;
	}
	y = static_cast<int>(abs(yRanged) > 0.5f);
	if (yRanged < 0)
	{
		y *= -1;
	}

Every direction vector is then multiplied with a seperation strength and applied to the position of the room.

![seperation](https://user-images.githubusercontent.com/35343159/211347278-b68dee9a-3ce3-47fb-9a8d-e24d54c803e5.gif)

**Other approaches**  
Detecting which rooms to separate could also be done in other ways. One such approach would be seperating from every room within a certain radius and then seperating more from rooms close by then seperating from rooms further away. I have tried this approach but it did not give the desired result for me.

Another approach to tackle the seperation, is by using physics bodies. These bodies will then push bodies which it is colliding with away, producing the same result as a seperation steering behavior.

**Complexity**  
One step of the seperation runs in O(N²) time, because every room needs to check every other room.  
This can be optimized by using spatial partitioning.

### Step 3: Discarding unusable rooms
During the third step, every room that is too small or is too close to another room, gets discarded.

The check if rooms are too close to each other runs in O(N²) time as well, which can also be optimized by using spatial partitioning.

![discardrooms](https://user-images.githubusercontent.com/35343159/211347310-7afd87ca-bafb-40fd-a444-2db81a4f8386.gif)


### Step 4: Triangulation
For the triangulation step I used the **Bowyer–Watson algorithm**, which is a method of for computing the *Delaunay triangulation*. I chose Delaunay triangulation mainly because it avoids sliver triangles. (triangles with 2 extremely acute angles)  
This algorithm is an *incremental algorithm*, which means it adds one vertex at a time and only retriangulates the affected parts of the graph.

Explanation of the algorithm:

 1. Start with an empty list of triangles  
 2. Create a super triangle that contains every point you want to add  
 3. Add vertices one by one    
	 3.1. Find every triangle for which the vertex is inside the circumcircle  
	 3.2. Remove these intersecting triangles from the list  
	 3.3. Find the boundary of the polygon formed by the removed triangles  
	 3.4. Retriangulate the polygon by creating triangles from every edge of the polygon to the new vertex  
4. Remove any triangle that contains a vertex from the super triangle  

![triangulation](https://user-images.githubusercontent.com/35343159/211347329-0b19e6fc-0e27-45dc-8cc8-f7637f83033b.gif)


### Step 5: Minimum spanning tree
A minimum spanning tree is a subset of a connected weighted undirected graph. This subset contains every vertex of the original graph without cycles, where the sum of the total edge weight has been minimized.    


I implemented **Kruskal's algorithm**, which is a greedy algorithm that finds the minimum spanning forest.  
Because I know that my graph will be connected, this algorithm will find the minimum spanning tree, because the graph only has one component. 

Explanation of the algorithm:
*(using V as the amount of vertices)*
 
 1. Start with an empty list of trees
 2. Sort all edges of the graph ascending by weight
 3. While there are still edges to check and the MST does not contain V-1 edges  
     3.1. Get the edge with minimum weight  
     3.2. If this edge creates a cycle in a tree, discard it  
     3.3. If this edge connects two trees, then combine the two trees and add the edge  
     3.4. If this edge connects to a single tree, add the edge to the tree

![kruskal](https://user-images.githubusercontent.com/35343159/211562822-41ec0674-95cd-47d6-ad8a-be4cb58aa57c.gif)

 **Complexity**  
 This algorithm runs in O(E log E) time, with E being the amount of edges in the graph.  
 
 This is achieved by:
  1. Sorting the edges by weight, which takes O(E log E) time because I used an std::set. Insertion in an std::set has a worst-case scenario of O(log E). With E insertion this brings us to O(E log E).
  2. Sorting the edges allows step 4, get the edge with minimum weight, to run in O(1) time.
  3. Each tree has a set of indices of the rooms connected to the tree. This allows searching if an edge (2 indices) is connected to a tree to run in O(log V) time, with V being the amount of vertices in the tree. This means that in a worst-case scenario in which we have to loop over all edges, this runs in O(E log V) time.


### Step 6: Generate corridors
To generate corridors I loop over each edge of the minimum spanning tree created in the previous step.  
I calculate if the room is a vertical or horizontal corridor, and depending on vertical or horizontal, I then clamp the size of the corridor so it doesn't overlap with the rooms it is bordering with.

![generatecorridors](https://user-images.githubusercontent.com/35343159/211562863-6b84e4db-7616-4e45-b009-0a2a5ca56770.png)

## Sources
https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm  
https://en.wikipedia.org/wiki/Delaunay_triangulation  
https://theor.xyz/mapgen/delaunay-triangulation-triangle-storage/  
https://en.wikipedia.org/wiki/Minimum_spanning_tree  
https://en.wikipedia.org/wiki/Kruskal%27s_algorithm
