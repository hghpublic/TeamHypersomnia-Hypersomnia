#include "image.h"
#include "log.h"


namespace augs {

	struct posrgba
	{
		vec2i pos;
		rgba col;
	};

	std::vector<vec2i> image::get_polygonized() const {

		std::vector<vec2i> vertices;
		std::vector<bool> pixelFields;
		pixelFields.resize(size.area(), false);
		for (int i = 0;i < size.h;++i)
		{
			for (int j = 0;j < size.w;++j)
			{
				rgba p = pixel(i, j);
				if (p == red)
				{
					vertices.push_back(vec2i(i, j));
					break;
				}
			}
			if (vertices.size() != 0)
				break;
		}
		if (vertices.size() == 0)
		{
			LOG("SMH WRONG FAM");
		}
		pixelFields[vertices.back().y * size.w + vertices.back().x] = true;
		vec2i field;
		field = vertices[0];
		vec2i offsets[8] = { vec2i(1,0),vec2i(0,1),vec2i(-1,0),vec2i(0,-1),vec2i(1,-1),vec2i(1,1),vec2i(-1,1),vec2i(-1,-1) }; //CLOCKWISE, PRIORITAL
		bool quit = false;
		do
		{
			for (int i = 0;i < 8;++i)
			{
				posrgba current;
				current.pos = field + offsets[i];
				if (current.pos.x >= size.h || current.pos.x < 0 || current.pos.y >= size.w
					|| current.pos.y < 0)
					continue;
				if (vertices.size() > 2 && current.pos == vertices[0])
				{
					quit = true;
					continue;
				}
				if (pixelFields[current.pos.y * size.w + current.pos.x])
				{
					continue;
				}
				current.col = pixel(current.pos);
				if (current.col == black)
				{
					field = current.pos;
					pixelFields[current.pos.y * size.w + current.pos.x] = true;
					break;
				}	
				else if (current.col == red)
				{
					vertices.push_back(current.pos);
					field = current.pos;
					pixelFields[current.pos.y * size.w + current.pos.x] = true;
					break;
				}
				pixelFields[current.pos.y * size.w + current.pos.x] = true;
				if (current.pos == vec2i(0, 0))
					quit = true;
			}	
		}while (!quit);
		
		return vertices;	
	}	

}