#include <iterator>

#include "data_zone.hpp"

void DataZone::divide_zone( int zone_index, int separation_index )
{
	auto& zone = zones[ zone_index ];
	auto& separation = separations[ separation_index ];
	int index = 0;
	
	while( !boost::geometry::equals( zone[index], separation[0] ) && index < static_cast<int>( zone.size() ) )
		++index;
							
	ring ring_one{{ zone[index] }};
	point p = zone[index];
	int cpt = index;
	int loop = 0;
	while( !boost::geometry::equals( p, separation[1] ) && loop < zone.size() )
	{
		++cpt;
		++loop;
		if( cpt >= static_cast<int>( zone.size() ) )
			cpt = 0;
		p = zone[cpt];
		boost::geometry::append( ring_one, p );
	}

	boost::geometry::correct( ring_one );

	index = 0;
	while( !boost::geometry::equals( zone[index], separation[1] ) && index < static_cast<int>( zone.size() ) )
		++index;

	ring ring_two{{ zone[index] }};
	loop = 0;
	while( !boost::geometry::equals( p, separation[0] ) && loop < zone.size() )
	{
		++cpt;
		++loop;
		if( cpt >= static_cast<int>( zone.size() ) )
			cpt = 0;
		p = zone[cpt];
		boost::geometry::append( ring_two, p );
	}

	boost::geometry::correct( ring_two );

	auto iterator = zones.begin();
	std::advance( iterator, zone_index );
	zones.erase( iterator );
	zones.push_back( ring_one );
	zones.push_back( ring_two );	
}

void DataZone::merge_zones( int separation_index )
{
	auto& separation = separations[ separation_index ];

	ring zone1;
	ring zone2;

	int index1 = -1;
	int index2 = -1;
	
	for( size_t k = 0 ; k < zones.size() ; ++k )
	{
		if( boost::geometry::covered_by( separation, zones[k] ) )
			if( index1 == -1 )
			{
				zone1 = zones[k];
				index1 = k;
			}
			else
			{
				zone2 = zones[k];
				index2 = k;
				break;
			}
	}

	// we want zone1 and zone2 such that zone1[0] == separation[0] and zone2[0] == separation[1]
	if( boost::geometry::equals( zone2[0], separation[0] ) )
		std::swap( zone1, zone2 );

	ring merged_zone;

	for( size_t i = 0 ; i < zone1.size() - 1 ; ++i )
		boost::geometry::append( merged_zone, zone1[i] );

	for( size_t i = 1 ; i < zone2.size() ; ++i )
		boost::geometry::append( merged_zone, zone2[i] );
	
	auto iterator = zones.begin();
	std::advance( iterator, index2 );
	zones.erase( iterator );
	iterator = zones.begin();
	std::advance( iterator, index1 );
	zones.erase( iterator );

	zones.push_back( merged_zone );
}

DataZone::DataZone( const std::vector<ghost::Variable>& variables, const polygon& contour, const std::vector<multi_point>& resources, const std::vector<line>& separations )
	: AuxiliaryData( variables ),
	  contour{ contour },
	  resources{ resources },
	  separations{ separations },
	  separation_lengths( std::vector<double>( separations.size() ) ),
	  zones{ {{contour.outer()}} }
{
	for( size_t i = 0 ; i < separations.size() ; ++i )
		separation_lengths[i] = boost::geometry::length( separations[i] );
	
	for( size_t i = 0 ; i < variables.size() ; ++i )
		if( variables[i].get_value() == 1 )
		{
			for( size_t k = 0 ; k < zones.size() ; ++k )
			{
				if( boost::geometry::covered_by( separations[i], zones[k] ) )
				{
					divide_zone( k, i );
					break;
				}
			}				
		}
}

void DataZone::required_update( const std::vector<ghost::Variable*>& variables, int index, int new_value )
{
	if( variables[index]->get_value() == 0 && new_value == 1 )
		for( size_t k = 0 ; k < zones.size() ; ++k )
		{
			if( boost::geometry::covered_by( separations[index], zones[k] ) )
			{
				divide_zone( k, index );
				break;
			}
		}

	if( variables[index]->get_value() == 1 && new_value == 0 )
		merge_zones( index );
}
