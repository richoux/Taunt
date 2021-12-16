#pragma once

#include <memory>

#include <ghost/objective.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using line = boost::geometry::model::linestring<point>;
using ring = boost::geometry::model::ring<point>;
using polygon = boost::geometry::model::polygon<point>;

class SameArea : public ghost::Minimize
{
	polygon _contour;
	std::vector<line> _separations;

	bool is_in( const line& separation, const ring& zone ) const;

public:
	SameArea( const std::vector<ghost::Variable>& variables,
	          const polygon& contour,
	          const std::vector<line>& separations );
	
	double required_cost( const std::vector<ghost::Variable*>& variables ) const override;
};
