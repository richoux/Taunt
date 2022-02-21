#include "model/constraint_no_cuts.hpp"

NoCuts::NoCuts( const std::vector<ghost::Variable>& variables,
                const boost_polygon& contour,
                const std::vector<multipoint>& resources,
                const std::vector<line>& separations )
	: Constraint( variables ),
	  _contour( contour ),
	  _separations( separations )
{
	for( auto& resource : resources )
	{
		if( !boost::geometry::is_empty( resource ) )
		{
			box box;
			boost::geometry::envelope( resource, box );
			_resource_boxes.push_back( box );
		}
	}
}

double NoCuts::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	int error = 0;

	std::vector<line> vec_lines;
	
	for( size_t i = 0 ; i < variables.size() ; ++i )
	{
		if( variables[i]->get_value() == 1 )
		{
			// Don't cross resource boxes
			for( auto& resource_box : _resource_boxes )
				if( boost::geometry::intersects( _separations[i], resource_box ) )
					++error;

			// Don't cross unwalkable/unbuildable parts
			for( auto& inner : _contour.inners() )
				if( boost::geometry::crosses( _separations[i], inner ) || boost::geometry::covered_by( _separations[i], inner ) )
					++error;

			// Don't cross separations each other
			for( size_t j = i + 1 ; j < variables.size() ; ++j )
				if( variables[j]->get_value() == 1 && boost::geometry::crosses( _separations[i], _separations[j] ) )
					++error;
		}
	}

	return error;
}
