#ifndef BVH_HPP
#define BVH_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <utility>

struct AABB
{
	int minX;
	int maxX;

	int minY;
	int maxY;

	int minZ;
	int maxZ;
};

class TrianglesCluster
{
	public:

		TrianglesCluster(int aCapacity = 100);
		void cut();
		AABB & getAABB();
		std::vector<Vertex> & getVertices();
		std::vector<int> & getIndices();

	private:

		int capacity; // maximum amount of triangles a cluster can store

		AABB bbox;
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		Material& material;

		std::shared_ptr<TrianglesCluster> frontBotLeft;
		std::shared_ptr<TrianglesCluster> frontBotRight;
		std::shared_ptr<TrianglesCluster> frontTopLeft;
		std::shared_ptr<TrianglesCluster> frontTopRight;

		std::shared_ptr<TrianglesCluster> backBotLeft;
		std::shared_ptr<TrianglesCluster> backBotRight;
		std::shared_ptr<TrianglesCluster> backTopLeft;
		std::shared_ptr<TrianglesCluster> backTopRight;
};

class BVH
{
	public:
		BVH();

	private:

		AABB bbox;
		std::vector<std::shared_ptr<BVH>> children;
};

#endif
