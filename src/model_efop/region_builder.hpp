#pragma once

#include <vector>

#include <ghost/model_builder.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;
using multi_point = boost::geometry::model::multi_point<point>;

class RegionBuilder : public ghost::ModelBuilder
{
	int _number_separations;
	polygon _contour;
	std::vector<multi_point> _resources;
	
	public:
	RegionBuilder( int number_separations,
	               const polygon& contour,
	               const std::vector<multi_point>& resources );

	void declare_variables() override;
	void declare_constraints() override;
	void declare_objective() override;
};
