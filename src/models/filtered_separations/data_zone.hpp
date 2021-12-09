#pragma once

#include <vector>
#include <ghost/auxiliary_data.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;
using multi_point = boost::geometry::model::multi_point<point>;
using ring = boost::geometry::model::ring<point>;
using line = boost::geometry::model::linestring<point>;

class DataZone : public ghost::AuxiliaryData
{
	void divide_zone( int zone_index, int separation_index );
	void merge_zones( int separation_index );
	
public:
	polygon contour;
	std::vector<multi_point> resources;
	std::vector<line> separations;
	std::vector<double> separation_lengths;
	std::vector<ring> zones;
	
	DataZone( const std::vector<ghost::Variable>& variables, const polygon& contour, const std::vector<multi_point>& resources, const std::vector<line>& separations );

	void required_update( const std::vector<ghost::Variable*>& variables, int index, int new_value ) override;
};
