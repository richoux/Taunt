#pragma once

#include <vector>

#include <ghost/model_builder.hpp>
#include <ghost/thirdparty/randutils.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;
using multi_point = boost::geometry::model::multi_point<point>;
using ring = boost::geometry::model::ring<point>;
using line = boost::geometry::model::linestring<point>;

class RegionBuilder : public ghost::ModelBuilder
{
	randutils::mt19937_rng _rng;
	
	public:
	RegionBuilder( int number_separations,
	               const polygon& contour,
	               const std::vector<multi_point>& resources );

	void declare_variables() override;
	void declare_constraints() override;
	void declare_objective() override;
	//void declare_auxiliary_data() override;

	int number_separations;
	polygon contour;
	std::vector<multi_point> resources;
	std::vector<line> separation_candidates;
};
