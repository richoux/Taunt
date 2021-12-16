#pragma once

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using multi_point = boost::geometry::model::multi_point<point>;
using line = boost::geometry::model::linestring<point>;
using ring = boost::geometry::model::ring<point>;
using polygon = boost::geometry::model::polygon<point>;

class OneClusterPerRegion : public ghost::Constraint
{
	polygon _contour;
	std::vector<multi_point> _resources;
	int _number_separations;
	int _zone_area;
	int _target_area;

public:
	OneClusterPerRegion( const std::vector<ghost::Variable>& variables,
	                     const polygon& contour,
	                     const std::vector<multi_point>& resources,
	                     int number_separations );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
