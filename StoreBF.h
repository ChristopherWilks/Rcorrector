#ifndef _MOURISL_CLASS_STORE_BF
#define _MOURISL_CLASS_STORE_BF
/**
  The wrapper for kmers' storage.
*/
#include <stdio.h>
#include <stdint.h>

#include "KmerCode.h"
#include "bf.h"

class StoreBF
{
private:
	uint64_t size ;
	static const uint64_t width=4;
	static const uint64_t num_hashes=3;
	bf::spectral_mi_bloom_filter** filters;

	int* sizes;
	int* heights;
	
	int num_filters;

	//all Morris approx. counting related code is adapted from the SF paper/codebase for CML
	double morris_base;
	uint64_t* limits;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution;
	
	bool morris_choice(int cur_count)
	{
		double r = distribution(generator);
		double lim = pow(morris_base, -cur_count);
		return r < lim;
	}

	double morris_point_query(int cur_count)
	{
		return cur_count == 0 ? 0 : pow(morris_base, cur_count - 1);
	}
	
	uint64_t tomb_query(uint64_t val, int* filter_layer, bool unique=true)
	{
		int i;
		uint64_t running_count = 0;
		for(i=0;i<num_filters;i++)
		{
			//TODO: make sure this returns 0 for non-existing values
			size_t c = filters[i]->lookup( val );
			running_count += c;
			if(c < limits[i])
			{
				*filter_layer = i;
				break;
			}
			else if(i==num_filters-1)
			{
				fprintf(stderr,"still -1\n");
				exit(-1);
			}
		}
		return running_count;
	}

	int tomb_insert(uint64_t val)
	{
		int filter_layer = -1;
		uint64_t count = tomb_query(val, &filter_layer, false);
		//all filled or there's a weird error
		if(filter_layer == -1)
		{
			fprintf(stderr,"weird error returning false in tomb_insert\n");
			exit(-1);
		}
		bool added = 0;
		if(morris_choice(count))
		{
			filters[filter_layer]->add( val );
			added = 1;
		}
		return added;
	}

public:
	
	StoreBF()
	{
		num_filters = 4;
		morris_base = 1.04;
		int sizes_[4] = {1000,500,100,100};
		int heigths_[4] = {16,16,16,16};

		sizes = &sizes_[0];	
		heights = &heigths_[0];
		fprintf(stderr,"num_filters %d base %f\n",num_filters,morris_base);
		filters = new bf::spectral_mi_bloom_filter*[num_filters];
		limits = new uint64_t[num_filters];
		int i;
		for(i=0;i<num_filters;i++)
		{
			fprintf(stderr,"size %d height %d\n",sizes[i],heights[i]);
			filters[i] = new bf::spectral_mi_bloom_filter(bf::make_hasher(num_hashes), sizes[i], heights[i]);
			//limits[i] = pow(morris_base, heights[i]);
			limits[i] = pow(2, heights[i]);
		}
		//cbf = bf::spectral_mi_bloom_filter(bf::make_hasher(num_hashes), s, width);
	}
	
	int Put( KmerCode &code, int cnt ) 
	{
		if ( !code.IsValid() )
			return 0 ;
		int i;
		for(i=0;i<cnt;i++)
		{
			tomb_insert( code.GetCanonicalKmerCode() ) ;
		}
		return 0;
	}

	

	int GetCount( KmerCode &code ) 
	{
		if ( !code.IsValid() )
			return 0 ;
		//return IsIn( code.GetCode(), code.GetKmerLength() ) ;
		return (int) TOMB_query( code );	
	}

	uint64_t TOMB_query( KmerCode &code )
	{
		if ( !code.IsValid() )
			return 0 ;
		int filter_layer = -1;
		uint64_t running_count = tomb_query(code.GetCode(), &filter_layer);
		if(filter_layer == -1)
			return -1;
	//for querying with Morris, use SF:CML code:
	//smallest_counter <= 1 ? morris_point_query(smallest_counter) : (int)(round((1 - morris_point_query(smallest_counter + 1)) / (1 - morris_base)));
		//TODO: need to double check this update which takes the filter_layer(s) into consideration to get accurate count since each level is an order of magnitude
		//uint64_t cur_layer_count = count <= 1 ? (uint64_t)morris_point_query(count) : (uint64_t)(round((1 - morris_point_query(count + 1)) / (1 - morris_base)));
		//if(filter_layer > 0)
		//	cur_layer_count += pow(morris_base, filter_layer);
		//THIS seems more in line with the Van Durme paper
		//the counts across all layers' slots are simply added together *then* they're used as the morris exponent
		uint64_t cur_count = running_count <= 1 ? (uint64_t)morris_point_query(running_count) : (uint64_t)(round((1 - morris_point_query(running_count + 1)) / (1 - morris_base)));
		return cur_count;
	}

	~StoreBF() 
	{
	}

	void Allocate( uint64_t cnt )
	{
	}

	uint64_t GetCanonicalKmerCode( uint64_t code, int k ) 
	{
		int i ;
		uint64_t crCode = 0ull ; // complementary code
		for ( i = 0 ; i < k ; ++i )
		{
			//uint64_t tmp = ( code >> ( 2ull * (k - i - 1) ) ) & 3ull   ; 
			//crCode = ( crCode << 2ull ) | ( 3 - tmp ) ;

			uint64_t tmp = ( code >> ( 2ull * i ) ) & 3ull ;
			crCode = ( crCode << 2ull ) | ( 3ull - tmp ) ;
		}
		return crCode < code ? crCode : code ;
	}

	int Clear() 
	{
		return 0 ;
	}
} ;
#endif
