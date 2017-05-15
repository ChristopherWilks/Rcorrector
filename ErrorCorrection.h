#ifndef _RCORRECTOR_ERRORCORRECTION_MOURISL
#define _RCORRECTOR_ERRORCORRECTION_MOURISL

#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "Store.h"
#include "StoreBF.h"
#include "Reads.h"
#include "utils.h"

struct _ErrorCorrectionThreadArg
{
	int kmerLength ;
	StoreBF *kmers ;
	struct _Read *readBatch, *readBatch2 ;
	int batchSize ;
	int batchUsed ;

	bool interleaved ;
	
	pthread_mutex_t *lock ;
} ;

void *ErrorCorrection_Thread( void *arg )  ;
int GetStrongTrustedThreshold( char *seq, char *qual, KmerCode &kcode, StoreBF &kmers ) ;
int ErrorCorrection( char *id, char *seq, char *qual, int pairStrongTrustThreshold, KmerCode &kcode, StoreBF &kmers ) ;
void GetKmerInformation( char *seq, int kmerLength, StoreBF &kmers, int &l, int &m, int &h ) ;

#endif
