#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>
#include <algorithm>

#include "model/constraint_one_cluster_per_region.hpp"

OneClusterPerRegion::OneClusterPerRegion( const std::vector<ghost::Variable>& variables,
                                          const boost_polygon& polygon,
                                          const std::vector<multipoint>& resources,
                                          const std::vector<line>& separations )
	: Constraint( variables ),
	  _polygon( polygon ),
	  _separations( separations )
{
	for( auto& resource : resources )
	{
		if( !boost::geometry::is_empty( resource ) )
		{
			box box;
			boost::geometry::envelope( resource, box );
			ring box_ring;
			boost::geometry::convert( box, box_ring );
			boost::geometry::correct( box_ring );
			_resource_boxes.push_back( box_ring );
		}
	}
}

bool OneClusterPerRegion::on_the_border( const line& separation, const ring& zone ) const
{
	point start = separation[0];
	point end = separation[1];

	for( int i = 0 ; i < static_cast<int>( zone.size() ) ; ++i )
		if( boost::geometry::equals( zone[i], start ) )
		{
			int i_plus = i == zone.size() - 1 ? 1 : i+1;
			int i_minus = i == 0 ? zone.size() - 2 : i-1;

			return ( boost::geometry::equals( zone[i_plus], end ) || boost::geometry::equals( zone[i_minus], end ) );
		}

	return false;
}

bool OneClusterPerRegion::is_in( const line& separation, const ring& zone ) const
{
	bool has_start = false;
	bool has_end = false;

	for( auto& p : zone )
	{
		if( boost::geometry::equals( p, separation[0] ) )
			has_start = true;
		if( boost::geometry::equals( p, separation[1] ) )
			has_end = true;
	}

	return has_start && has_end;
}

double OneClusterPerRegion::required_error( const std::vector<ghost::Variable*>& variables ) const
{
#if defined MUF
	std::cout << "Call OneClusterPerRegion::required_error\n";
#endif

	std::vector<ring> zones{ {{_polygon.outer()}} };

	for( size_t i = 0 ; i < variables.size() ; ++i )
		if( variables[i]->get_value() == 1 )
			for( size_t k = 0 ; k < zones.size() ; ++k )
				if( is_in( _separations[i], zones[k] ) )
				{
					int separation_index = i;
					int zone_index = k;
					auto& zone = zones[ zone_index ];
					auto& separation = _separations[ separation_index ];
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
							cpt = 1; // not cpt = 0, otherwise we will repeat twice the same point, since zone[0]==zone[size-1]
						p = zone[cpt];
						boost::geometry::append( ring_one, p );
					}
					boost::geometry::correct( ring_one );

					ring ring_two{{ zone[index], p }};
					loop = 0;
					while( !boost::geometry::equals( p, separation[0] ) && loop < zone.size() )
					{
						++cpt;
						++loop;
						if( cpt >= static_cast<int>( zone.size() ) )
							cpt = 1; // not cpt = 0, otherwise we will repeat twice the same point, since zone[0]==zone[size-1]
						p = zone[cpt];
						boost::geometry::append( ring_two, p );
					}
					boost::geometry::correct( ring_two );

#if defined POUET
					std::cout << "\nBefore division\n";
					for( size_t k = 0 ; k < zones.size() ; ++k )
						std::cout << "Zone[" << k << "]: " << boost::geometry::dsv( zones[k] ) << "\n";

					std::cout << "Separation " << boost::geometry::dsv( separation ) << "\n";
					std::cout << "Zone " << boost::geometry::dsv( zone ) << "\n";
					std::cout << "Ring one " << boost::geometry::dsv( ring_one ) << "\n";
					std::cout << "Ring two " << boost::geometry::dsv( ring_two ) << "\n";
#endif
	
					auto iterator = zones.begin();
					std::advance( iterator, zone_index );
					zones.erase( iterator );
					zones.push_back( ring_one );
					zones.push_back( ring_two );

#if defined POUET
					std::cout << "After division\n";
					for( size_t k = 0 ; k < zones.size() ; ++k )
						std::cout << "Zone[" << k << "]: " << boost::geometry::dsv( zones[k] ) << "\n";
#endif
					break;
				}

	std::vector<int> nb_clusters_in_zone( zones.size(), 0 );
	std::vector<int> nb_zones_for_cluster( _resource_boxes.size(), 0 );
	for( size_t b = 0 ; b < _resource_boxes.size() ; ++b )
	{
#if defined MUF
		std::cout << "Cluster box " << boost::geometry::dsv( _resource_boxes[b] ) << "\n";
#endif
		
		for( size_t z = 0 ; z < zones.size() ; ++z )
			if( boost::geometry::overlaps( _resource_boxes[b], zones[z] ) || boost::geometry::covered_by( _resource_boxes[b], zones[z] ) )
			{
#if defined MUF
				std::cout << "Cluster box " << boost::geometry::dsv( _resource_boxes[b] )
				          << " in zone " << boost::geometry::dsv( zones[z] ) << "\n";
#endif
				++nb_clusters_in_zone[z];
				++nb_zones_for_cluster[b];
			}
	}

#if defined MUF
	for( size_t z = 0 ; z < zones.size() ; ++z )
		if( nb_clusters_in_zone[z] == 0 )
		{
			std::cout << "No clusters covered by zone " << boost::geometry::dsv( zones[z] ) << "\n";
			for( auto& box : _resource_boxes )
				std::cout << "Cluser box " << boost::geometry::dsv( box ) << "\n";
		}
#endif	

	return std::max( 0, *std::max_element( nb_clusters_in_zone.begin(), nb_clusters_in_zone.end() ) - 1 )
		+ std::count_if( nb_zones_for_cluster.begin(), nb_zones_for_cluster.end(), [&](auto& c){ return c == 0 ;} );
}
