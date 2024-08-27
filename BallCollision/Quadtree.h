#pragma once
#include <vector>

struct Rect
{
	int x, y, width, height;
	Rect(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
};

class IQuadFitable
{
public:
	virtual bool IsFitsTheRect(const Rect&) const = 0;
};

constexpr int MAX_OBJECTS_PER_QUADTREE = 5;
constexpr int MAX_QUADTREE_DEPTH = 4;

class Quadtree
{
	int depth;
	std::vector<Quadtree> children;
	std::vector<IQuadFitable*> objects;
	Rect rect;

public:
	Quadtree(const Rect& rect, int depth = 0) : rect(rect), depth(depth) {}

	const std::vector<Quadtree>& GetChildren() const { return children; }
	const Rect& GetRect() const { return rect; }

	bool Insert(IQuadFitable* obj)
	{
		// try to insert in the deepest quadtree
		for (Quadtree& quadtree : children)
		{
			if (quadtree.Insert(obj))
			{
				return true;
			}
		}

		// skip if object doesnt fit in
		if (!obj->IsFitsTheRect(rect))
			return false;

		objects.push_back(obj);
		if (children.size() == 0 && objects.size() > MAX_OBJECTS_PER_QUADTREE && depth < MAX_QUADTREE_DEPTH)
		{
			// quadtree is oversized - split it in 4
			Split();

			// spread objects across created subquads
			objects.erase(
				std::remove_if(objects.begin(), objects.end(),
					[&](IQuadFitable* obj)
					{
						for (auto& subquad : children)
						{
							if (obj->IsFitsTheRect(subquad.rect))
							{
								subquad.objects.push_back(obj);
								return true;
							}
						}
						return false;
					}
				),
				objects.end()
			);
		}
		return true;
	}

	// Splits quadtree into 4 subtrees
	void Split()
	{
		int subWidth = rect.width / 2;
		int subHeight = rect.height / 2;
		int x = rect.x;
		int y = rect.y;

		children.push_back(Quadtree(Rect(x, y, subWidth, subHeight), depth + 1));
		children.push_back(Quadtree(Rect(x + subWidth, y, subWidth, subHeight), depth + 1));
		children.push_back(Quadtree(Rect(x, y + subHeight, subWidth, subHeight), depth + 1));
		children.push_back(Quadtree(Rect(x + subWidth, y + subHeight, subWidth, subHeight), depth + 1));
	}

	void Clear()
	{
		for (Quadtree& quadtree : children)
			quadtree.Clear();

		objects.clear();
		children.clear();
	}
};