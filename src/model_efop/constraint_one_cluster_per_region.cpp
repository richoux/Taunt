#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>

#include "constraint_one_cluster_per_region.hpp"

OneClusterPerRegion::OneClusterPerRegion( const std::vector<ghost::Variable>& variables,
                                          const polygon& contour,
                                          const std::vector<multi_point>& resources,
                                          int number_separations )
	: Constraint( variables ),
	  _contour( contour ),
	  _resources( resources ),
	  _number_separations( number_separations ),
	  _zone_area( boost::geometry::area( _contour.outer() ) ),
	  _target_area( std::max( 150, _zone_area / ( _number_separations + 5 ) ) ) // say we need 2 separations, so target areas should be at least 1/7th of the total area
{ }

double OneClusterPerRegion::required_error( const std::vector<ghost::Variable*>& variables ) const
{	
	std::vector<bool> processed( variables.size(), false );
	std::vector<ring> subregions;
	subregions.push_back( _contour.outer() );
	int error = 0;

	//std::cout << "Initial ring:\n" << boost::geometry::dsv(_contour.outer()) << "\n\n";

	for( size_t i = 0 ; i < variables.size() - 2 ; ++i )
	{
		if( variables[i]->get_value() > 0 && !processed[i] )
		{
			processed[i] = true;
			for( size_t j = i + 2 ; j < variables.size() ; ++j )
				if( variables[j]->get_value() == variables[i]->get_value() && !processed[j] )
				{
					processed[j] = true;
				
					line separation{ _contour.outer()[i] , _contour.outer()[j] };
					//std::cout << "Found separation " << boost::geometry::dsv(separation) << "\n";

					bool found_subregion = false;
					for( int k = static_cast<int>( subregions.size() ) - 1 ; k >= 0 && !found_subregion ; --k )
					{
						if( boost::geometry::covered_by( separation, subregions.at(k) ) )
						{
							//std::cout << "Separation " << boost::geometry::dsv(separation) << " covered by ring " << boost::geometry::dsv(subregions.at(k)) << "\n";

							int index = 0;
							while( !boost::geometry::equals( subregions.at(k)[index], _contour.outer()[i] ) && index < static_cast<int>( subregions.at(k).size() ) )
								++index;

							// if( index == static_cast<int>( subregions.at(k).size() ) )
							// {
							// 	std::cout << "Looking for point index " << boost::geometry::dsv(_contour.outer()[i]) << " in ring:\n"
							// 	          << boost::geometry::dsv( subregions.at(k) ) << "\n";
							// }
							
							ring ring_one{{ subregions.at(k)[index] }};
							point p;
							int cpt = index;
							int loop = 0;
							while( !boost::geometry::equals( p, _contour.outer()[j] ) && loop < subregions.at(k).size() )
							{
								++cpt;
								++loop;
								if( cpt >= static_cast<int>( subregions.at(k).size() ) )
									cpt = 0;
								p = subregions.at(k)[cpt];
								boost::geometry::append( ring_one, p );
							}

							// if( loop == static_cast<int>( subregions.at(k).size() ) )
							// {
							// 	std::cout << "Looking for point " << boost::geometry::dsv(_contour.outer()[j]) << " in ring:\n"
							// 	          << boost::geometry::dsv( subregions.at(k) ) << "\n";
							// }

							boost::geometry::correct( ring_one );
							//std::cout << "Subring one: " << boost::geometry::dsv(ring_one) << "\n";

							index = 0;
							while( !boost::geometry::equals( subregions.at(k)[index], _contour.outer()[j] ) && index < static_cast<int>( subregions.at(k).size() ) )
								++index;

							// if( index == static_cast<int>( subregions.at(k).size() ) )
							// {
							// 	std::cout << "Looking for point index " << boost::geometry::dsv(_contour.outer()[j]) << " in ring:\n"
							// 	          << boost::geometry::dsv( subregions.at(k) ) << "\n";
							// }

							ring ring_two{{ subregions.at(k)[index] }};
							loop = 0;
							while( !boost::geometry::equals( p, _contour.outer()[i] ) && loop < subregions.at(k).size() )
							{
								++cpt;
								++loop;
								if( cpt >= static_cast<int>( subregions.at(k).size() ) )
									cpt = 0;
								p = subregions.at(k)[cpt];
								boost::geometry::append( ring_two, p );
							}

							// if( loop == static_cast<int>( subregions.at(k).size() ) )
							// {
							// 	std::cout << "Looking for point " << boost::geometry::dsv(_contour.outer()[i]) << " in ring:\n"
							// 	          << boost::geometry::dsv( subregions.at(k) ) << "\n";
							// }

							boost::geometry::correct( ring_two );
							// std::cout << "Subring two: " << boost::geometry::dsv(ring_two) << "\n";

							auto iterator = subregions.begin();
							std::advance( iterator, k );
							subregions.erase( iterator );
							subregions.push_back( ring_one );
							subregions.push_back( ring_two );
							found_subregion = true;
						}
					}				
				}
		}
	}

	// std::string regionfilename{ "maps/taunted/andromeda_region.svg" };	
	// std::ofstream regionfile_svg( regionfilename );
	// boost::geometry::svg_mapper<point> mapper( regionfile_svg, 8000, 8000 );
	// for( size_t i = 0 ; i < subregions.size() ; ++i )
	// 	mapper.add( subregions[i] );

	// for( int i = 0 ; i < static_cast<int>( subregions.size() ) ; ++i )
	// {
	// 	int color = std::min( 255, 50 + i * 50 );
	// 	std::string svg_cmd_first = "fill-opacity:0.5;fill:rgb(";
	// 	std::string svg_cmd_second = ",0,0);stroke:rgb(0,0,0);stroke-width:5";
	// 	std::string svg_cmd =  svg_cmd_first + std::to_string( color ) + svg_cmd_second;
	// 	mapper.map( subregions[i], svg_cmd );
	// }
	
	std::vector<point> cluster_centers;
	for( auto& resource : _resources )
	{
		point center;
		boost::geometry::centroid( resource, center );
		cluster_centers.push_back( center );
	}
	
	for( auto& subregion : subregions )
	{
		bool found_center = false;

		for( auto& center : cluster_centers )
			if( boost::geometry::covered_by( center, subregion ) )
				if( found_center )
				{
					++error;
					
					if( boost::geometry::area( subregion ) < _target_area )
						++error;
				}
				else
				{
					found_center = true;
					
					std::cout << "Zone area: " << _zone_area
					          << ", target area: " << _target_area
					          << ", region area: " << boost::geometry::area( subregion ) << "\n";

					if( boost::geometry::area( subregion ) < _target_area )
						++error;
				}
		
		if( !found_center )
			++error;
	}

	return error;
}
