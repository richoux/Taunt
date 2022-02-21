#pragma once

#include <memory>

#include <ghost/objective.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

#include "../geometry.hpp"
//using point = boost::geometry::model::d2::point_xy<int>;
//using line = boost::geometry::model::linestring<point>;

class MinSeparationWidth : public ghost::Minimize
{
	std::vector<double> _separation_lengths;
	
public:
	MinSeparationWidth( const std::vector<ghost::Variable>& variables,
	                    const std::vector<line>& separations );
	
	double required_cost( const std::vector<ghost::Variable*>& variables ) const override;
};
