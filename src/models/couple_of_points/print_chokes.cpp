#include <iomanip>

#include "print_chokes.hpp"

PrintChokes::PrintChokes( const polygon& contour )
	: _contour( contour )
{ }
	
std::stringstream PrintChokes::print_candidate( const std::vector<ghost::Variable>& variables ) const
{
	std::stringstream ss;
	
	// for( int i = 0 ; i < static_cast<int>( variables.size() ) ; ++i )
	// {
	// 	ss << boost::geometry::dsv( _contour.outer()[i] ) << " ";
	// 	if( i != 0 && i % 10 == 0 )
	// 		ss << "\n";
	// }

	// ss << "\n";

	for( int i = 0 ; i < static_cast<int>( variables.size() ) ; ++i )
		if( variables[i].get_value() > 0 )
			ss << "Choke " << variables[i].get_value() << ", point " << boost::geometry::dsv( _contour.outer()[i] ) << "\n";
	
	return ss;
}
