#ifndef _MOURISL_CLASS_STORE_CMF
#define _MOURISL_CLASS_STORE_CMF
/**
  The wrapper for kmers' storage.
*/
#include <stdio.h>
#include <stdint.h>

#include "KmerCode.h"
#include "cmlsketch.h"
#include "bench_common.h"

const static int SF_LENGTH=8;

class StoreCML
{
private:
	uint64_t size ;
	CMLSketch* cml;

public:
	StoreCML(CMLSketch* cml_)
	{
		cml = cml_;
	}
	
	//following functions from:
	//http://stackoverflow.com/questions/9695720/how-do-i-convert-a-64bit-integer-to-a-char-array-and-back
	//needed it to convert from Lighter's uint64 to character array for SF sketch
	void int64ToChar(unsigned char a[], int64_t n) {
		  memcpy(a, &n, 8);
	}

	int64_t charTo64bitNum(unsigned char a[]) {
		  int64_t n = 0;
		  memcpy(&n, a, 8);
		  return n;
	}

	~StoreCML() 
	{
	}

	void Allocate( uint64_t cnt )
	{
	}

	int Put( KmerCode &code, int cnt  ) 
	{
		if ( !code.IsValid() )
			return 0 ;
		uint64_t val = code.GetCanonicalKmerCode();
		//hash[ code.GetCanonicalKmerCode() ] = cnt ;
		unsigned char valc[SF_LENGTH];	
		int64ToChar(valc, val);
          	cml->insert(valc, SF_LENGTH, cnt);
		return 0 ;
	}

	int GetCount( KmerCode &code ) 
	{
		if ( !code.IsValid() )
			return 0 ;
		uint64_t val = code.GetCanonicalKmerCode();
		unsigned char valc[SF_LENGTH];	
		int64ToChar(valc, val);
       		return cml->queryPoint(valc, SF_LENGTH);
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
