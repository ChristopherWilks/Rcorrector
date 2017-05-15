#ifndef _MOURISL_CLASS_STORE_CQF
#define _MOURISL_CLASS_STORE_CQF
/**
  The wrapper for kmers' storage.
*/
#include <stdio.h>
#include <stdint.h>

#include "KmerCode.h"
#include "gqf.h"
#include "hashutil.h"

class StoreCQF
{
private:
	uint64_t size ;
	QF cqf;
	int qbits;
	int rbits;
	uint64_t num_hash_bits;
	uint64_t n_unique_kmers;

public:
	StoreCQF()
	{
		qbits = 25; //# of distinct kmers dependent //20 is example default
		rbits = 17; //default from squeakr //8 is example default
		num_hash_bits = qbits + rbits;
		n_unique_kmers = (uint64_t) pow(2.0, 30.0);
		uint32_t seed = time(NULL);
		qf_init(&cqf, (1ULL<<qbits), num_hash_bits, 0, true, "", seed);
	}
	
	StoreCQF( uint64_t cnt, double fp=0.01 )
	{
		num_hash_bits = (uint64_t) log10(cnt/fp)/log10(2);
		qbits = log10(cnt)/log10(2); //# of distinct kmers dependent //20 is example default
		rbits = num_hash_bits - qbits; //default from squeakr //8 is example default
		n_unique_kmers = cnt;
		uint32_t seed = time(NULL);
		qf_init(&cqf, (1ULL<<qbits), num_hash_bits, 0, true, "", seed);
	}

	~StoreCQF() 
	{
		qf_destroy(&cqf, true);
	}

	void Allocate( uint64_t cnt )
	{
	}

	int Put( KmerCode &code, int cnt  ) 
	{
		if ( !code.IsValid() )
			return 0 ;
		uint64_t val = code.GetCanonicalKmerCode();
		val = kmercounting::HashUtil::MurmurHash64A(((void*)&val), sizeof(val), cqf.metadata->seed);
		qf_insert(&cqf, val % cqf.metadata->range, 0, cnt, false, false);
		//hash[ code.GetCanonicalKmerCode() ] = cnt ;
		return 0 ;
	}

	int GetCount( KmerCode &code ) 
	{
		if ( !code.IsValid() )
			return 0 ;
		uint64_t val = code.GetCanonicalKmerCode();
		val = kmercounting::HashUtil::MurmurHash64A(((void*)&val), sizeof(val), cqf.metadata->seed);
		return qf_count_key_value(&cqf, val % cqf.metadata->range, 0);
		//return hash[ code.GetCanonicalKmerCode() ] ;
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
