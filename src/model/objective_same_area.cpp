#include "model/objective_same_area.hpp"

SameArea::SameArea( const std::vector<ghost::Variable>& variables, const polygon& contour, const std::vector<line>& separations )
	: Minimize( variables, "Least Squares Area Difference" ),
	  _contour( contour ),
	  _separations( separations )
{ }

bool SameArea::is_in( const line& separation, const ring& zone ) const
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


double SameArea::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	std::vector<ring> zones{ {{_contour.outer()}} };

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
	
					auto iterator = zones.begin();
					std::advance( iterator, zone_index );
					zones.erase( iterator );
					zones.push_back( ring_one );
					zones.push_back( ring_two );

					break;
				}

	std::vector<double> areas;
	for( auto& region : zones )
		areas.push_back( boost::geometry::area( region ) );

	double mean_area = std::accumulate( areas.begin(), areas.end(), 0 ) / areas.size();
	double least_squares = 0.0;
	for( auto& area : areas )
		least_squares += std::pow( mean_area - area, 2 );
		
	return least_squares;
}
