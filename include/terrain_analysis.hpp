#pragma once
#include <vector>

#include "analyze_type.hpp"
#include "region.hpp"

namespace taunt
{
	class terrain_analysis
	{
	  analyze_type _analyze_type;
	  std::vector< region > _regions;

	public:
	  terrain_analysis();
	  terrain_analysis( analyze_type at );

	  void analyze();
	  void change_analyze_type( analyze_type at );
	};
}
