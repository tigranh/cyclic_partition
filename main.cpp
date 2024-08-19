
#include <functional>
#include <algorithm>
#include <chrono>
#include <random>
#include <iostream>
#include <vector>
#include <cassert>

#include "partition_hoare.hpp"
#include "partition_cyclic.hpp"

#include "large_object.hpp"


/// Runs general tests on provided function, which is expected to partition 
/// given sequence.
template< typename BidIt, typename Pred, typename Gen >
void test_partition_on_int_sequence(
		BidIt (*partition_f)( BidIt begin, BidIt end, Pred pred ), 
		Gen& gen )
{
	// Type of the predicate, used in partitioning
	typedef std::binder2nd< std::less< int > > pred_t;

	// Test on ordinary sequences
	{
		int a[] = { 5, 12, 9, 3, 6, 14, 18, 7 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 5 );  // Size of left part should be '5'
		assert( std::is_partitioned( a, a+n, pred ) );
	}
	{
		int a[] = { 20, 32, 14, 6, 8, 9, 3 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 4 );  // Size of left part should be '4'
		assert( std::is_partitioned( a, a+n, pred ) );
	}
	{
		int a[] = { 4, 8, 12, 16, 25 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 2 );  // Size of left part should be '2'
		assert( std::is_partitioned( a, a+n, pred ) );
	}
	{
		int a[] = { 25, 32, 14, 19, 62, 66, 92, 4, 18 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				20 );  // the predicate is "x < 20"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 4 );  // Size of left part should be '4'
		assert( std::is_partitioned( a, a+n, pred ) );
	}

	// Test when left part is empty
	{
		int a[] = { 12, 14, 22, 20, 19 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 0 );  // Size of left part should be '0'
		assert( std::is_partitioned( a, a+n, pred ) );
	}

	// Test when right part is empty
	{
		int a[] = { 8, 2, 4, 6 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + n );  // Size of left part should be 'n'
		assert( std::is_partitioned( a, a+n, pred ) );
	}

	// Test on empty sequence
	{
		int a[] = { 0 };
		const int n = 0;
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 0 );  // Size of left part should be '0'
	}

	// Test on 1-long sequences
	{
		int a[] = { 6 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 1 );  // Size of left part should be '1'
		assert( std::is_partitioned( a, a+n, pred ) );
	}
	{
		int a[] = { 14 };
		const int n = sizeof(a) / sizeof(a[0]);
		pred_t pred( 
				std::less< int >(), 
				10 );  // the predicate is "x < 10"

		int* mid = (*partition_f)( a, a+n, pred );
		assert( mid == a + 0 );  // Size of left part should be '0'
		assert( std::is_partitioned( a, a+n, pred ) );
	}

	// Perform random tests
	{
		const int n = 100;  // Length of the array
		int a[ n ];
		std::uniform_int_distribution<> values_dist( 0, 10'000 );
		pred_t pred( 
				std::less< int >(), 
				5'000 );  // the predicate is "x < 5'000"
		const int T = 100;  // Number of random tests
		for ( int t = 0; t < T; ++t ) {
			// Generate random array
			std::generate( 
					a, 
					a+n, 
					std::bind( values_dist, gen ) );
			// Partition it
			(*partition_f)( a, a+n, pred );
			// Check
			assert( std::is_partitioned( a, a+n, pred ) );
		}
	}

}


/// The clock-type, used to measure performance.
typedef std::chrono::high_resolution_clock clock_type;

// A global "collector" variable, which will accumulate results of 
// runs of all searches, in order to avoid compiler optimizations.
volatile unsigned long long collector = 0;


/// Runs provided partitioning function 'partition_f', for 'T' times, 
/// each time on random array of length 'N', with integer values uniformly 
/// distributed in range [min_value, max_value], while partitioning 
/// by pivot value 'pivot'.
/// Measures and prints overall time (in milliseconds), spend on all those 
/// partitioning calls.
template< typename Gen >
clock_type::duration run_partitioning_on_integers( 
		int N, 
		int min_value, int max_value, 
		int pivot, 
		int* (*partition_f)( int*, int*, std::binder2nd< std::less< int > > ), 
		int T, 
		Gen& gen )
{
	std::uniform_int_distribution<> values_dist( min_value, max_value );
			// Random values from range [min_value, max_value]
	std::vector< int > a( N );
			// Partitioning array of length 'N'
	std::generate( a.begin(), a.end(), 
			std::bind( values_dist, gen ) );
			// Generating the random array
	typedef std::binder2nd< std::less< int > > pred_t;
			// Type of predicate, used for partitioning
	pred_t pred( std::less< int >(), pivot );
			// Predicate is "x < pivot"
	std::uniform_int_distribution<> offset_dist( 0, N-1 );
			// Random offsets, for cyclic rotation of array, between 
			// adjacent calls to partition
	clock_type::duration overall_duration( 0 );
	for ( int t = 0; t < T; ++t ) {
		// Call the partitioning function
		clock_type::time_point start_time = clock_type::now();
		int* mid = (*partition_f)( 
				a.data(), 
				a.data() + N, 
				pred );
		clock_type::time_point end_time = clock_type::now();
		overall_duration += (end_time - start_time);
		// Somehow alter the global collector variable
		collector += (mid - a.data());
		// Cyclic rotate, to prepare for the next partitioning call
		int offset = offset_dist( gen );
		std::rotate( 
				a.begin(), 
				a.begin() + offset, 
				a.end() );
	}
	std::cout << std::chrono::duration_cast< std::chrono::milliseconds >( overall_duration ).count()
			<< " msc" << std::endl;
	return overall_duration;
}


/// Runs provided partitioning function 'partition_f', for 'T' times, 
/// each time on a random array of length 'N', with uniformly distributed 
/// "large objects" inside it. After partitioning, ratio of left part and 
/// right part will be [left_ratio, 1.0 - left_ratio].
/// Measures and prints overall time (in milliseconds), spend on all those 
/// partitioning calls.
template< typename LargeObject, typename Gen >
clock_type::duration run_partitioning_on_large_objects( 
		int N, 
		double left_ratio, 
		LargeObject* (*partition_f)( LargeObject*, LargeObject*, std::binder2nd< std::less< LargeObject > > ),
		int T, 
		Gen& gen )
{
	std::vector< LargeObject > a( N );
			// Partitioning array of length 'N'
	generate_random_large_objects_sequence< LargeObject >(
			N, 
			a.begin(), 
			gen );  // Generating the random array
	const LargeObject pivot = generate_pivot_large_object< LargeObject >(
			left_ratio, 
			gen );  // Generate the pivot "large object"
	typedef std::binder2nd< std::less< LargeObject > > pred_t;
			// Type of predicate, used for partitioning
	pred_t pred( std::less< LargeObject >(), pivot );
			// The predicate is "x < pivot"
	std::uniform_int_distribution<> offset_dist( 0, N-1 );
			// Random offsets, for cyclic rotation of array, between 
			// adjacent calls to partition
	clock_type::duration overall_duration( 0 );
	for ( int t = 0; t < T; ++t ) {
		// Call the partitioning function
		clock_type::time_point start_time = clock_type::now();
		LargeObject* mid = (*partition_f)( 
				a.data(), 
				a.data() + N, 
				pred );
		clock_type::time_point end_time = clock_type::now();
		overall_duration += (end_time - start_time);
		// Somehow alter the global collector variable
		collector += (mid - a.data());
		// Cyclic rotate, to prepare for the next partitioning call
		int offset = offset_dist( gen );
		std::rotate( 
				a.begin(), 
				a.begin() + offset, 
				a.end() );
	}
	std::cout << std::chrono::duration_cast< std::chrono::milliseconds >( overall_duration ).count()
			<< " msc" << std::endl;
	return overall_duration;
}


int main( int argc, char* argv[] )
{
	/// The random numbers engine, used through entire executable.
	std::default_random_engine gen;

	std::cout << " --- Testing partition algorithms --- " << std::endl;

	{
		typedef std::binder2nd< std::less< int > > pred_t;
				// Type of predicate, used for partitioning

		std::cout << "\t partition_hoare< int*, ... >() ..." << std::endl;
		test_partition_on_int_sequence( 
				& ml::algorithm::partition_hoare< int*, pred_t >, 
				gen );

		std::cout << "\t partition_cyclic< int*, ... >() ..." << std::endl;
		test_partition_on_int_sequence( 
				& ml::algorithm::partition_cyclic< int*, pred_t >, 
				gen );
	}

	std::cout << " --- Benchmarking partition algorithms --- " << std::endl;

	{
		std::cout << "partitioning on arrays of integers ..." << std::endl;

		typedef std::binder2nd< std::less< int > > pred_t;
				// Type of predicate, used for partitioning

		const int N = 1'000'000;   // Length of the array of integers
		const int T = 500;         // Number of runs
		std::cout << "\t Array length : " << N << std::endl;
		std::cout << "\t Number of runs : " << T << std::endl;

		// left/right lengths ratio - 1:1
		std::cout << "\t left/right lengths ratio (after partitioning) - 1:1" << std::endl;
		std::cout << "\t\t Hoare scheme : ";
		run_partitioning_on_integers(
				N, 
				0, 50'000'000, 
				25'000'000, 
				& ml::algorithm::partition_hoare< int*, pred_t >, 
				T, 
				gen );
		std::cout << "\t\t Cyclic partition : ";
		run_partitioning_on_integers(
				N, 
				0, 50'000'000, 
				25'000'000, 
				& ml::algorithm::partition_cyclic< int*, pred_t >, 
				T, 
				gen );

		// left/right lengths ratio - 1:3
		std::cout << "\t left/right lengths ratio (after partitioning) - 1:3" << std::endl;
		std::cout << "\t\t Hoare scheme : ";
		run_partitioning_on_integers(
				N, 
				0, 50'000'000, 
				12'500'000, 
				& ml::algorithm::partition_hoare< int*, pred_t >, 
				T, 
				gen );
		std::cout << "\t\t Cyclic partition : ";
		run_partitioning_on_integers(
				N, 
				0, 50'000'000, 
				12'500'000, 
				& ml::algorithm::partition_cyclic< int*, pred_t >, 
				T, 
				gen );
	}

	{
		std::cout << "partitioning on arrays of \"large objects\" ..." << std::endl;

		typedef large_object< unsigned short, 256 > large_object_t;
				// What we take as "large object"
		typedef std::binder2nd< std::less< large_object_t > > pred_t;
				// The predicate, used for comparison of "large objects"

		const int N = 25'000;    // Length of the array of "large objects"
		const int T = 500;       // Number of runs
		std::cout << "\t Array length : " << N << std::endl;
		std::cout << "\t Number of runs : " << T << std::endl;

		// left/right lengths ratio - 1:1
		std::cout << "\t left/right lengths ratio (after partitioning) - 1:1" << std::endl;
		std::cout << "\t\t Hoare scheme : ";
		run_partitioning_on_large_objects(
				N, 
				0.5, 
				& ml::algorithm::partition_hoare< large_object_t*, pred_t >, 
				T, 
				gen );
		std::cout << "\t\t Cyclic partition : ";
		run_partitioning_on_large_objects(
				N, 
				0.5, 
				& ml::algorithm::partition_cyclic< large_object_t*, pred_t >, 
				T, 
				gen );

		// left/right lengths ratio - 1:3
		std::cout << "\t left/right lengths ratio (after partitioning) - 1:3" << std::endl;
		std::cout << "\t\t Hoare scheme : ";
		run_partitioning_on_large_objects(
				N, 
				0.25, 
				& ml::algorithm::partition_hoare< large_object_t*, pred_t >, 
				T, 
				gen );
		std::cout << "\t\t Cyclic partition : ";
		run_partitioning_on_large_objects(
				N, 
				0.25, 
				& ml::algorithm::partition_cyclic< large_object_t*, pred_t >, 
				T, 
				gen );
	}

	std::cout << "Final value of the 'collector' variable (to prevent compiler optimizations): " 
			<< collector << std::endl;

	return 0;
}

