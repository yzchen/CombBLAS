
#ifdef THREADED
#ifndef _OPENMP
#define _OPENMP
#endif

#include <omp.h>
int cblas_splits = 1;
#endif

#include "../include/CombBLAS.h"
#include <mpi.h>
#include <sys/time.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <limits>


#include "BPMaximalMatching.h"
#include "BPMaximumMatching.h"
#include "ApproxWeightPerfectMatching.h"

using namespace std;

namespace combblas {

// algorithmic options
bool prune,randMM, moreSplit;
int init;
bool randMaximal;
bool fewexp;
bool randPerm;
bool saveMatching;
string ofname;


typedef SpParMat < int64_t, bool, SpDCCols<int64_t,bool> > Par_DCSC_Bool;
typedef SpParMat < int64_t, int64_t, SpDCCols<int64_t, int64_t> > Par_DCSC_int64_t;
typedef SpParMat < int64_t, double, SpDCCols<int64_t, double> > Par_DCSC_Double;
typedef SpParMat < int64_t, double, SpCCols<int64_t, double> > Par_CSC_Double;
typedef SpParMat < int64_t, bool, SpCCols<int64_t,bool> > Par_CSC_Bool;

template <class IT, class NT, class DER>
void TransformWeight(SpParMat < IT, NT, DER > & A, bool applylog)
{
	//A.Apply([](NT val){return log(1+abs(val));});
	// if the matrix has explicit zero entries, we can still have problem.
	// One solution is to remove explicit zero entries before cardinality matching (to be tested)
	//A.Apply([](NT val){if(val==0) return log(numeric_limits<NT>::min()); else return log(fabs(val));});
	A.Apply([](NT val){return (fabs(val));});
	
	FullyDistVec<IT, NT> maxvRow(A.getcommgrid());
	A.Reduce(maxvRow, Row, maximum<NT>(), static_cast<NT>(numeric_limits<NT>::lowest()));
	A.DimApply(Row, maxvRow, [](NT val, NT maxval){return val/maxval;});
	
	FullyDistVec<IT, NT> maxvCol(A.getcommgrid());
	A.Reduce(maxvCol, Column, maximum<NT>(), static_cast<NT>(numeric_limits<NT>::lowest()));
	A.DimApply(Column, maxvCol, [](NT val, NT maxval){return val/maxval;});
	
    if(applylog)
        A.Apply([](NT val){return log(val);});

}
void ShowUsage()
{
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    if(myrank == 0)
    {
        cout << "\n-------------- usage --------------\n";
        cout << "Usage: ./awpm -input <filename>\n";
        cout << "Optional parameters: -randPerm: randomly permute the matrix for load balance (default: no random permutation)\n";
        cout << "                     -optsum: Optimize the sum of diagonal (default: Optimize the product of diagonal)\n";
        cout << "                     -noWeightedCard: do not use weighted cardinality matching (default: use weighted cardinality matching)\n";
        cout << "                     -output <output file>: output file name (if not provided: inputfile.awpm.txt)\n";
        cout << " \n-------------- examples ----------\n";
        cout << "Example: mpirun -np 4 ./awpm -input cage12.mtx \n" << endl;
        cout << "(output matching is saved to cage12.mtx.awpm.txt)\n" << endl;
    }
}







int main(int argc, char* argv[])
{
    
    // ------------ initialize MPI ---------------
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
    if (provided < MPI_THREAD_SERIALIZED)
    {
        printf("ERROR: The MPI library does not have MPI_THREAD_SERIALIZED support\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int nprocs, myrank;
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    if(argc < 3)
    {
        ShowUsage();
        MPI_Finalize();
        return -1;
    }
    
    init = DMD;
    randMaximal = false;
    prune = false;
    randMM = true;
    moreSplit = false;
    fewexp=false;
    saveMatching = true;
    ofname = "";
    randPerm = false;
    bool optimizeProd = true; // by default optimize sum_log_abs(aii) (after equil)
    
    bool weightedCard = true;
    string ifilename = "";
    string ofname = "";
    for(int i = 1; i<argc; i++)
    {
        if (string(argv[i]) == string("-input")) ifilename = argv[i+1];
        if (string(argv[i]) == string("-output")) ofname = argv[i+1];
        if (string(argv[i]) == string("-optsum")) optimizeProd = false;
        if (string(argv[i]) == string("-noWeightedCard")) weightedCard = false;
        if (string(argv[i]) == string("-randPerm")) randPerm = true;
    }
    if(ofname=="") ofname = ifilename + ".awpm.txt";

    
    
    
    // ------------ Process input arguments and build matrix ---------------
    {
        Par_DCSC_Double * AWeighted;
        ostringstream tinfo;
        tinfo << fixed;
        cout << fixed;
        double t01, t02;
        if(ifilename!="")
        {
            AWeighted = new Par_DCSC_Double();
            t01 = MPI_Wtime();
            AWeighted->ParallelReadMM(ifilename, true, maximum<double>()); // one-based matrix market file
            t02 = MPI_Wtime();
       
            if(AWeighted->getnrow() != AWeighted->getncol())
            {
                 SpParHelper::Print("Rectangular matrix: Can not compute a perfect matching.\n");
                MPI_Finalize();
                return -1;
            }
            
            tinfo.str("");
            tinfo << "Reading input matrix in" << t02-t01 << " seconds" << endl;
            SpParHelper::Print(tinfo.str());
            
            SpParHelper::Print("Pruning explicit zero entries....\n");
            AWeighted->Prune([](double val){return fabs(val)==0;}, true);
            
            AWeighted->PrintInfo();
        }
        else
        {
            ShowUsage();
            MPI_Finalize();
            return -1;
        }
        
        
        
        // ***** careful: if you permute the matrix, you have the permute the matching vectors as well!!
        // randomly permute for load balance
        
        FullyDistVec<int64_t, int64_t> randp( AWeighted->getcommgrid());
        if(randPerm)
        {
            if(AWeighted->getnrow() == AWeighted->getncol())
            {
                randp.iota(AWeighted->getnrow(), 0);
                randp.RandPerm();
                (*AWeighted)(randp,randp,true);
                SpParHelper::Print("Matrix is randomly permuted for load balance.\n");
            }
            else
            {
                SpParHelper::Print("Rectangular matrix: Can not apply symmetric permutation.\n");
            }
        }
        
       
        Par_DCSC_Bool A = *AWeighted; //just to compute degree
        // Reduce is not multithreaded, so I am doing it here
        FullyDistVec<int64_t, int64_t> degCol(A.getcommgrid());
        A.Reduce(degCol, Column, plus<int64_t>(), static_cast<int64_t>(0));
       
        
        // transform weights
        if(optimizeProd)
            TransformWeight(*AWeighted, true);
        else
            TransformWeight(*AWeighted, false);
        // convert to CSC for SpMSpV calls
        Par_CSC_Double AWeightedCSC(*AWeighted);
        Par_CSC_Bool ABoolCSC(*AWeighted);
        
        
		// Compute the initial trace
        int64_t diagnnz;
        double origWeight = Trace(*AWeighted, diagnnz);
        bool isOriginalPerfect = diagnnz==A.getnrow();
       

        
        

        FullyDistVec<int64_t, int64_t> mateRow2Col ( A.getcommgrid(), A.getnrow(), (int64_t) -1);
        FullyDistVec<int64_t, int64_t> mateCol2Row ( A.getcommgrid(), A.getncol(), (int64_t) -1);
        

      
        
        init = DMD; randMaximal = false; randMM = false; prune = true;
        
        // Maximal
        double ts = MPI_Wtime();
        if(weightedCard)
            WeightedGreedy(AWeightedCSC, mateRow2Col, mateCol2Row, degCol);
        else
            WeightedGreedy(ABoolCSC, mateRow2Col, mateCol2Row, degCol);
        double tmcl = MPI_Wtime() - ts;
        
		double mclWeight = MatchingWeight( *AWeighted, mateRow2Col, mateCol2Row);
        SpParHelper::Print("After Greedy sanity check\n");
		bool isPerfectMCL = CheckMatching(mateRow2Col,mateCol2Row);
        
        if(isOriginalPerfect && mclWeight<=origWeight) // keep original
        {
            SpParHelper::Print("Maximal is not better that the natural ordering. Hence, keeping the natural ordering.\n");
            mateRow2Col.iota(A.getnrow(), 0);
            mateCol2Row.iota(A.getncol(), 0);
            mclWeight = origWeight;
            isPerfectMCL = true;
        }
        
		
        // MCM
        double tmcm = 0;
        double mcmWeight = mclWeight;
        if(!isPerfectMCL) // run MCM only if we don't have a perfect matching
        {
            ts = MPI_Wtime();
            if(weightedCard)
                maximumMatching(AWeightedCSC, mateRow2Col, mateCol2Row, true, false, true);
            else
                maximumMatching(AWeightedCSC, mateRow2Col, mateCol2Row, true, false, false);
            tmcm = MPI_Wtime() - ts;
            mcmWeight =  MatchingWeight( *AWeighted, mateRow2Col, mateCol2Row) ;
            SpParHelper::Print("After MCM sanity check\n");
            CheckMatching(mateRow2Col,mateCol2Row);
        }
        
        
        // AWPM
        ts = MPI_Wtime();
        TwoThirdApprox(*AWeighted, mateRow2Col, mateCol2Row);
        double tawpm = MPI_Wtime() - ts;
        
		double awpmWeight =  MatchingWeight( *AWeighted, mateRow2Col, mateCol2Row) ;
        SpParHelper::Print("After AWPM sanity check\n");
        CheckMatching(mateRow2Col,mateCol2Row);
        if(isOriginalPerfect && awpmWeight<origWeight) // keep original
        {
            SpParHelper::Print("AWPM is not better that the natural ordering. Hence, keeping the natural ordering.\n");
            mateRow2Col.iota(A.getnrow(), 0);
            mateCol2Row.iota(A.getncol(), 0);
            awpmWeight = origWeight;
        }
        

        int nthreads = 1;
#ifdef THREADED
#pragma omp parallel
        {
            nthreads = omp_get_num_threads();
        }
#endif

        tinfo.str("");
        tinfo  << "Weight: [ Original Greedy MCM AWPM] " << origWeight << " " << mclWeight << " "<< mcmWeight << " " << awpmWeight << endl;
        tinfo <<  "Time: [Processes Threads Cores Greedy MCM AWPM Total] " << nprocs << " " << nthreads << " " << nprocs * nthreads << " " << tTotalMaximal << " "<< tTotalMaximum << " " << tawpm << " "<< tTotalMaximal + tTotalMaximum + tawpm << endl;
        SpParHelper::Print(tinfo.str());
        
        //revert random permutation if applied before
        if(randPerm==true && randp.TotalLength() >0)
        {
            // inverse permutation
            FullyDistVec<int64_t, int64_t>invRandp = randp.sort();
            mateRow2Col = mateRow2Col(invRandp);
        }
        if(saveMatching && ofname!="")
        {
            mateRow2Col.ParallelWrite(ofname,false,false);
        }
        
        
    }
    MPI_Finalize();
    return 0;
}

}

