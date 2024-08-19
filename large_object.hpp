
#ifndef ML__LARGE_OBJECT_HPP
#define ML__LARGE_OBJECT_HPP

#include <array>
#include <random>


// Definitions


/// Range of the values, which can be written in cells of "large objects".
static const int g_large_object_min_value = 0;
static const int g_large_object_max_value = 9'999;


// Structure


/// Definition of type of "large object".
template< typename IntType, unsigned int L >
struct large_object : public std::array< IntType, L >
	{};

/// Equality check for "large object"s.
template< typename LargeObjectT >
inline bool operator==( const LargeObjectT& a, const LargeObjectT& b )
	{ return a[0] == b[0]; }  // Check only the first items

/// Order check for "large object"s.
template< typename LargeObjectT >
inline bool operator<( const LargeObjectT& a, const LargeObjectT& b )
	{ return a[0] < b[0]; }  // Compare only the first items


// Generation routines


/// Generates 'N' random "large objects", and writes them in the 
/// output iterator, starting from 'out'.
template< typename LargeObjectT, typename OutIt, typename Gen >
inline OutIt generate_random_large_objects_sequence( int N, OutIt out, Gen& gen )
{
	std::uniform_int_distribution<
					typename LargeObjectT::value_type > 
			dist( 
					g_large_object_min_value, 
					g_large_object_max_value );
			// The values which will be written in the large objects.
	while ( N-- ) {
		LargeObjectT obj;
		for ( int i = 0; i < obj.size(); ++i )
			obj[ i ] = dist( gen );
		*(out++) = obj;
	}
	return out;
}


/// Generates such "large object", which when used as pivot, 
/// partitions given sequence into proportions [left_ratio : 1.0 - left_ratio].
template< typename LargeObjectT, typename Gen >
inline LargeObjectT generate_pivot_large_object(
		double left_ratio, 
		Gen& gen )
{
	LargeObjectT result;
	//   prepare 1 random large object
	generate_random_large_objects_sequence< LargeObjectT >(
			1, 
			&result, 
			gen );
	//   alter it in the necessary way
	result[ 0 ] = g_large_object_min_value * (1.0 - left_ratio)
			+ g_large_object_max_value * left_ratio;
	return result;
}


#endif
