#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>

#include "constraint_minimal_region_area.hpp"

MinRegionArea::MinRegionArea( const std::vector<ghost::Variable>& variables,
                              const polygon& contour,
                              const std::vector<line>& separations,
                              int number_separations )
	: Constraint( variables ),
	  _contour( contour ),
	  _separations( separations ),
	  _target_area( std::max( 180, static_cast<int>( boost::geometry::area( _contour.outer() ) / ( number_separations + 6 ) ) ) ) // say we need 2 separations, so target areas should be at least 1/8th of the total area
{
#if defined MUF
	for( size_t i = 0 ; i < variables.size() ; ++i )
	{
		std::vector<ring> zones{ {{_contour.outer()}} };
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
	
		for( auto& zone : zones )
			if( is_in( _separations[i], zone ) && boost::geometry::area( zone ) < _target_area )
				std::cout << "zone " << boost::geometry::dsv( zone )
				          << ", separation[ " << i << "] " << boost::geometry::dsv( _separations[i] )
				          << ": area = " << boost::geometry::area( zone ) << ")\n";
	}
#endif
}

bool MinRegionArea::on_the_border( const line& separation, const ring& zone ) const
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

bool MinRegionArea::is_in( const line& separation, const ring& zone ) const
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

double MinRegionArea::required_error( const std::vector<ghost::Variable*>& variables ) const
{
#if defined MUF
	std::cout << "Call MinRegionArea::required_error\n";
#endif
	int error = 0;

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
	
#if defined MUF
	std::cout << "Areas:\n";
#endif
	for( auto& var : variables )
		if( var->get_value() == 1 )
			for( auto& zone : zones )
				if( is_in( _separations[var->get_id()], zone ) && boost::geometry::area( zone ) < _target_area )
				{
#if defined MUF
					std::cout << "Variable " << var->get_id() << " - "
					          << "zone " << boost::geometry::dsv( zone )
					          << ", separation " << boost::geometry::dsv( _separations[var->get_id()] )
					          << ": area = " << boost::geometry::area( zone )
					          << " (target=" << _target_area << ")\n";
#endif
					++error;
				}
	
	return error;
}
