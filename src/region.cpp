#include "region.hpp"
#include "base_location.hpp"
#include "separation.hpp"
#include "terrain_analysis.hpp"

using namespace taunt;

region::region( int id, terrain_analysis *_terrain_analysis, const boost_polygon& polygon )
	: _id( id ),
	_ta( _terrain_analysis ),
	_polygon( polygon )
{
	point boost_centroid;
	boost::geometry::centroid( _polygon, boost_centroid );
	_centroid = position{ static_cast<position_type>( std::round( boost_centroid.x() ) ), static_cast<position_type>( std::round( boost_centroid.y() ) ) };
}

std::vector< base_location > region::get_base_locations() const
{
	return _ta->get_base_locations();
}

const base_location& region::get_first_base_locations() const
{
	return _ta->get_base_location( 0 );
}

std::vector< separation > region::get_separations() const
{
	return _ta->get_separations();
}

position region::get_nearest_contour_position( const position& pos )
{
	point p{ static_cast<double>( pos.x ), static_cast<double>( pos.y ) };
	point closest_point;
	double min_distance = std::numeric_limits<double>::max();

	for( const auto& point : _polygon.outer() )
		if( min_distance > boost::geometry::comparable_distance( p, point ) )
		{
			min_distance = boost::geometry::comparable_distance( p, point );
			closest_point = point;
		}

	return {static_cast<position_type>( std::round( closest_point.x() ) ), static_cast<position_type>( std::round( closest_point.y() ) ) };
}

position region::get_entry_position( const separation& sep )
{
	if( std::find_if( _separations_indexes.begin(), _separations_indexes.end(), [&]( auto i ) {return sep.get_id() == _ta->get_separation(i).get_id(); }) == _separations_indexes.end() )
		return {-1,-1};

	multipolygon commun;
	boost::geometry::intersection( sep._polygon, _polygon, commun );
	point boost_centroid;

	// This will always consider the first intersection only. How to improve that?
	// For instance on Destination, a ^-shaped separating bridge has two different
	// entry points for its south entry. How to consider both?
	boost::geometry::centroid( commun[0], boost_centroid);

	return { static_cast<position_type>( std::round( boost_centroid.x() ) ), static_cast<position_type>( std::round( boost_centroid.y() ) ) };
}
