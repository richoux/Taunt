#pragma once

#include <memory>

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using multi_point = boost::geometry::model::multi_point<point>;
using line = boost::geometry::model::linestring<point>;
using ring = boost::geometry::model::ring<point>;
using polygon = boost::geometry::model::polygon<point>;
using box = boost::geometry::model::box<point>;

class OneClusterPerRegion : public ghost::Constraint
{
	polygon _contour;
	std::vector<line> _separations;
	std::vector<ring> _resource_boxes;

	bool on_the_border( const line& separation, const ring& zone ) const;
	bool is_in( const line& separation, const ring& zone ) const;
	
public:
	OneClusterPerRegion( const std::vector<ghost::Variable>& variables,
	                     const polygon& contour,
	                     const std::vector<multi_point>& resources,
	                     const std::vector<line>& separations );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
