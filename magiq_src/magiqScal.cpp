#include <iostream>
#include <functional>
#include <algorithm>
#include <sstream>
#include <vector>
#include <iterator>
#include <fstream>
#include "../magiq_include/magiqScal.h"

using namespace std;
using namespace combblas;

void resgen_l1(PSpMat::MPI_DCCols &m_40, PSpMat::MPI_DCCols &m_34, PSpMat::MPI_DCCols &m_23, PSpMat::MPI_DCCols &m_53,
               PSpMat::MPI_DCCols &m_15, PSpMat::MPI_DCCols &m_65) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    double t1_start = MPI_Wtime();
    clear_result_time();

    auto commGrid = m_40.getcommgrid();

    // m_40 becoms m_04
    m_40.Transpose();

    vector<IndexType> indl, indr, indj;

    vector<IndexType> order1 = {0, 1, 1, 0, 1, 1}, order2 = {0, 0, 0, 1, 0, 2, 1, 0}, order3 = {0, 0, 0, 1, 0, 2, 0, 3},
            order4 = {0, 0, 1, 0, 0, 1, 0, 2, 0, 3}, order5 = {0, 0, 0, 1, 1, 0, 0, 2, 0, 3, 0, 4};

    double t1_end = MPI_Wtime();
    if (myrank == 0) {
        cout << "begin result generation ......" << endl << flush;
        cout << "\ttranspose matrix and declarations take : " << (t1_end - t1_start) << " s\n" << endl << flush;
    }

    get_local_indices(m_40, indl);
    send_local_indices(commGrid, indl);
    m_40.FreeMemory();

    get_local_indices(m_34, indr);
    send_local_indices(commGrid, indr);
    m_34.FreeMemory();

    local_join(commGrid, indl, indr, 2, 2, 1, 1, order1, indj);
    local_redistribution(m_53, indj, 3, 1, indl);

    get_local_indices(m_53, indr);
    send_local_indices(commGrid, indr);
    m_53.FreeMemory();

    local_join(commGrid, indl, indr, 3, 2, 1, 1, order2, indj);
    local_redistribution(m_65, indj, 4, 3, indl);

    get_local_indices(m_65, indr);
    send_local_indices(commGrid, indr);
    m_65.FreeMemory();

    local_filter(commGrid, indl, indr, 4, 2, 3, 2, 1, 0, order3, indj);
    indl = indj;

    get_local_indices(m_15, indr);
    send_local_indices(commGrid, indr);
    m_15.FreeMemory();

    local_join(commGrid, indl, indr, 4, 2, 3, 1, order4, indj);
    local_redistribution(m_23, indj, 5, 2, indl);

    get_local_indices(m_23, indr);
    send_local_indices(commGrid, indr);
    m_23.FreeMemory();

    local_join(commGrid, indl, indr, 5, 2, 2, 1, order5, indj);
    send_local_results(commGrid, indj.size() / 6);
}

void lubm_l1(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);

    FullyDistVec<IndexType, ElementType> r_40(commWorld, G.getnrow(), 0), l_23(commWorld, G.getnrow(), 0), l_15(commWorld, G.getnrow(), 0);

    IndexType ind1 = 103594630, ind2 = 139306106, ind3 = 130768016;
    if (isPerm) {
        ind1 = nonisov[103594630];
        ind2 = nonisov[139306106];
        ind3 = nonisov[130768016];
    }
    r_40.SetElement(ind1, 17);
    l_23.SetElement(ind2, 1);
    l_15.SetElement(ind3, 1);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 1" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(4,0) = G ⊗ {1@(103594630,103594630)}*17" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_40(G);
    multDimApplyPrune(m_40, r_40, Column, true);
    m_40.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : m_(3,4) = G ⊗ m_(4, 0).D()*12" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_34(G);
    diagonalizeV(m_40, dm, Row, 12);
    multDimApplyPrune(m_34, dm, Column, true);
    m_34.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(2,3) = G.T() ⊗ m_(3,4).D()*17" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    auto m_23(G);
    diagonalizeV(m_34, dm, Row, 17);
    multDimApplyPrune(m_23, dm, Row, true);
    m_23.Transpose();
    m_23.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 4
    if (myrank == 0) {
        cout << "step 4 : m_(2,3) = {1@(139306106,139306106)} × m_(2,3)" << endl << flush;
    }
    double t4_start = MPI_Wtime();
    multDimApplyPrune(m_23, l_23, Row, false);
    m_23.PrintInfo();
    double t4_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 4 (Total) : " << (t4_end - t4_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 5
    if (myrank == 0) {
        cout << "step 5 : m_(5,3) = G.T ⊗ m_(2,3).T().D()*14" << endl << flush;
    }
    double t5_start = MPI_Wtime();
    auto m_53(G);
    diagonalizeV(m_23, dm, Column, 14);
    multDimApplyPrune(m_53, dm, Row, true);
    m_53.Transpose();
    m_53.PrintInfo();
    double t5_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 5 (Total) : " << (t5_end - t5_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 6
    if (myrank == 0) {
        cout << "step 6 : m_(1,5) = G.T() ⊗ m_(5,3).D()*17" << endl << flush;
    }
    double t6_start = MPI_Wtime();
    auto m_15(G); 
    diagonalizeV(m_53, dm, Row, 17);
    multDimApplyPrune(m_15, dm, Row, true);
    m_15.Transpose();
    m_15.PrintInfo();
    double t6_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 6 (Total) : " << (t6_end - t6_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 7
    if (myrank == 0) {
        cout << "step 7 : m_(1,5) = {1@(130768016,130768016)} × m_(1,5)" << endl << flush;
    }
    double t7_start = MPI_Wtime();
    multDimApplyPrune(m_15, l_15, Row, false);
    m_15.PrintInfo();
    double t7_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 7 (Total) : " << (t7_end - t7_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 8
    if (myrank == 0) {
        cout << "step 8 : m_(6,5) = G.T() ⊗ m_(1,5).T().D()*3" << endl << flush;
    }
    double t8_start = MPI_Wtime();
    auto m_65(G);
    diagonalizeV(m_15, dm, Column, 3);
    multDimApplyPrune(m_65, dm, Row, true);
    m_65.Transpose();
    m_65.PrintInfo();
    double t8_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 8 (Total) : " << (t8_end - t8_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 9
    if (myrank == 0) {
        cout << "step 9 : m_(6,5) = m_(3,4).T().D() × m_(6,5)" << endl << flush;
    }
    double t9_start = MPI_Wtime();
    diagonalizeV(m_34, dm, Column);
    multDimApplyPrune(m_65, dm, Row, false);
    m_65.PrintInfo();
    double t9_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 9 (Total) : " << (t9_end - t9_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 10
    if (myrank == 0) {
        cout << "step 10 : m_(3,4) = m_(3,4) × m_(6,5).D()" << endl << flush;
    }
    double t10_start = MPI_Wtime();
    diagonalizeV(m_65, dm, Row);
    multDimApplyPrune(m_34, dm, Column, false);
    m_34.PrintInfo();
    double t10_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 10 (Total) : " << (t10_end - t10_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 11
    if (myrank == 0) {
        cout << "step 11 : m_(5,3) = m_(6,5).T().D() × m_(5,3)" << endl << flush;
    }
    double t11_start = MPI_Wtime();
    diagonalizeV(m_65, dm, Column);
    multDimApplyPrune(m_53, dm, Row, false);
    m_53.PrintInfo();
    double t11_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 11 (Total) : " << (t11_end - t11_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 12
    if (myrank == 0) {
        cout << "step 12 : m_(3,4) = m_(5,3).T().D() × m_(3,4)" << endl << flush;
    }
    double t12_start = MPI_Wtime();
    diagonalizeV(m_53, dm, Column);
    multDimApplyPrune(m_34, dm, Row, false);
    m_34.PrintInfo();
    double t12_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 12 (Total) : " << (t12_end - t12_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 13
    if (myrank == 0) {
        cout << "step 13 : m_(4,0) = m_(3,4).T().D() × m_(4,0)" << endl << flush;
    }
    double t13_start = MPI_Wtime();
    diagonalizeV(m_34, dm, Column);
    multDimApplyPrune(m_40, dm, Row, false);
    m_40.PrintInfo();
    double t13_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 13 (Total) : " << (t13_end - t13_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double query_counting = MPI_Wtime();

    if (myrank == 0) {
        cout << "query1 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query1 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query1 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query1 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query1 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
    resgen_l1(m_40, m_34, m_23, m_53, m_15, m_65);
    double resgen_end = MPI_Wtime();

    // end count total time
    double total_computing_2 = MPI_Wtime();

    if (myrank == 0) {
        cout << "query1 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query1 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

void resgen_l2(PSpMat::MPI_DCCols &m_10, PSpMat::MPI_DCCols &m_21) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    double t1_start = MPI_Wtime();

    clear_result_time();

    auto commGrid = m_10.getcommgrid();

    // m_10 becoms m_01
    m_10.Transpose();

    vector<IndexType> indl, indr, indj;

    vector<IndexType> order1 = {0, 0, 0, 1, 1, 0};

    double t1_end = MPI_Wtime();
    if (myrank == 0) {
        cout << "begin result generation ......" << endl << flush;
        cout << "\ttranspose matrix and declarations take : " << (t1_end - t1_start) << " s\n" << endl << flush;
    }

    get_local_indices(m_10, indl);
    send_local_indices(commGrid, indl);
    m_10.FreeMemory();

    get_local_indices(m_21, indr);
    send_local_indices(commGrid, indr);
    m_21.FreeMemory();

    local_join(commGrid, indl, indr, 2, 2, 1, 1, order1, indj);
    send_local_results(commGrid, indj.size() / 3);
}

void lubm_l2(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);

    FullyDistVec<IndexType, ElementType> r_10(commWorld, G.getnrow(), 0);

    IndexType ind1 = 235928023;
    if (isPerm)
        ind1 = nonisov[235928023];

    r_10.SetElement(ind1, 17);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 2" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(1,0) = G ⊗ {1@(235928023,235928023)}*17" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_10(G);
    multDimApplyPrune(m_10, r_10, Column, true);
    m_10.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : m_(2,1) = G.T() ⊗ m_(1,0).D()*9" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_21(G);
    diagonalizeV(m_10, dm, Row, 9);
    multDimApplyPrune(m_21, dm, Row, true);
    m_21.Transpose();
    m_21.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(1,0) = m_(2,1).T().D() × m_(1,0)" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    diagonalizeV(m_21, dm, Column);
    multDimApplyPrune(m_10, dm, Row, false);
    m_10.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double query_counting = MPI_Wtime();

    if (myrank == 0) {
        cout << "query2 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query2 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query2 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query2 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query2 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
    resgen_l2(m_10, m_21);
    double resgen_end = MPI_Wtime();

    // end count time
    double total_computing_2 = MPI_Wtime();
    if (myrank == 0) {
        cout << "query2 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query2 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

void lubm_l3(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);

    FullyDistVec<IndexType, ElementType> r_40(commWorld, G.getnrow(), 0), l_23(commWorld, G.getnrow(), 0), l_15(commWorld, G.getnrow(), 0);

    IndexType ind1 = 103594630;
    IndexType ind2 = 223452631;
    IndexType ind3 = 130768016;

    if (isPerm) {
        ind1 = nonisov[103594630];
        ind2 = nonisov[223452631];
        ind3 = nonisov[130768016];
    }

    r_40.SetElement(ind1, 17);
    l_23.SetElement(ind2, 1);
    l_15.SetElement(ind3, 1);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 3" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(4,0) = G ⊗ {1@(103594630,103594630)}*17" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_40(G);
    multDimApplyPrune(m_40, r_40, Column, true);
    m_40.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : m_(3,4) = G ⊗ m_(4, 0).D()*12" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_34(G);
    diagonalizeV(m_40, dm, Row, 12);
    multDimApplyPrune(m_34, dm, Column, true);
    m_34.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(2,3) = G.T() ⊗ m_(3,4).D()*17" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    auto m_23(G);
    diagonalizeV(m_34, dm, Row, 17);
    multDimApplyPrune(m_23, dm, Row, true);
    m_23.Transpose();
    m_23.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 4
    if (myrank == 0) {
        cout << "step 4 : m_(2,3) = {1@(223452631,223452631)} × m_(2,3)" << endl << flush;
    }
    double t4_start = MPI_Wtime();
    multDimApplyPrune(m_23, l_23, Row, false);
    m_23.PrintInfo();
    double t4_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 4 (Total) : " << (t4_end - t4_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

//    // ==> step 5
//    if (myrank == 0) {
//        cout << "step 5 : m_(5,3) = G x m_(2,3).T().D()*14" << endl << flush;
//    }
//    double t5_start = MPI_Wtime();
//    diagonalizeV(m_23, dm, Column, 14);
//    multDimApplyPrune(m_53, dm, Column, true);
//    m_53.PrintInfo();
//    double t5_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 5 (Total) : " << (t5_end - t5_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 6
//    if (myrank == 0) {
//        cout << "step 6 : m_(1,5) = G.T() x m_(5,3).D()*17" << endl << flush;
//    }
//    double t6_start = MPI_Wtime();
//    diagonalizeV(m_53, dm, Row, 17);
//    multDimApplyPrune(m_15, dm, Column, true);
//    m_15.PrintInfo();
//    double t6_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 6 (Total) : " << (t6_end - t6_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 7
//    if (myrank == 0) {
//        cout << "step 7 : m_(1,5) = {1@(130768016,130768016)} x m_(1,5)" << endl << flush;
//    }
//    double t7_start = MPI_Wtime();
//    multDimApplyPrune(m_15, l_15, Row, false);
//    m_15.PrintInfo();
//    double t7_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 7 (Total) : " << (t7_end - t7_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 8
//    if (myrank == 0) {
//        cout << "step 8 : m_(6,5) = G.T() x m_(1,5).T().D()*3" << endl << flush;
//    }
//    double t8_start = MPI_Wtime();
//    diagonalizeV(m_15, dm, Column, 3);
//    multDimApplyPrune(m_65, dm, Column, true);
//    m_65.PrintInfo();
//    double t8_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 8 (Total) : " << (t8_end - t8_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 9
//    if (myrank == 0) {
//        cout << "step 9 : m_(6,5) = m_(3,4).T().D() x m_(6,5)" << endl << flush;
//    }
//    double t9_start = MPI_Wtime();
//    diagonalizeV(m_34, dm, Column);
//    multDimApplyPrune(m_65, dm, Row, false);
//    m_65.PrintInfo();
//    double t9_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 9 (Total) : " << (t9_end - t9_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 10
//    if (myrank == 0) {
//        cout << "step 10 : m_(3,4) = m_(3,4) x m_(6,5).D()" << endl << flush;
//    }
//    double t10_start = MPI_Wtime();
//    diagonalizeV(m_65, dm, Row);
//    multDimApplyPrune(m_34, dm, Column, false);
//    m_34.PrintInfo();
//    double t10_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 10 (Total) : " << (t10_end - t10_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 11
//    if (myrank == 0) {
//        cout << "step 11 : m_(5,3) = m_(6,5).T().D() x m_(5,3)" << endl << flush;
//    }
//    double t11_start = MPI_Wtime();
//    diagonalizeV(m_65, dm, Column);
//    multDimApplyPrune(m_53, dm, Row, false);
//    m_53.PrintInfo();
//    double t11_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 11 (Total) : " << (t11_end - t11_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 12
//    if (myrank == 0) {
//        cout << "step 12 : m_(3,4) = m_(5,3).T().D() x m_(3,4)" << endl << flush;
//    }
//    double t12_start = MPI_Wtime();
//    diagonalizeV(m_53, dm, Column);
//    multDimApplyPrune(m_34, dm, Row, false);
//    m_34.PrintInfo();
//    double t12_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 12 (Total) : " << (t12_end - t12_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }
//
//    // ==> step 13
//    if (myrank == 0) {
//        cout << "step 13 : m_(4,0) = m_(3,4).T().D() x m_(4,0)" << endl << flush;
//    }
//    double t13_start = MPI_Wtime();
//    diagonalizeV(m_34, dm, Column);
//    multDimApplyPrune(m_40, dm, Row, false);
//    m_40.PrintInfo();
//    double t13_end = MPI_Wtime();
//
//    if (myrank == 0) {
//        cout << "step 13 (Total) : " << (t13_end - t13_start) << " s" << endl << flush;
//        cout << "---------------------------------------------------------------" << endl << flush;
//    }

    double query_counting = MPI_Wtime();
    if (myrank == 0) {
        cout << "query3 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query3 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query3 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query3 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query3 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
#ifdef MAGIQ_DEBUG
    if (myrank == 0) {
        cout << "final size : 0" << endl << flush;
        cout << "total get local indices time : " << total_get_local_indices_time << " s" << endl << flush;
        cout << "total send local indices time : " << total_send_local_indices_time << " s" << endl << flush;
        cout << "total local join time : " << total_local_join_time << " s" << endl << flush;
        cout << "total local filter time : " << total_local_filter_time << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
#endif
    double resgen_end = MPI_Wtime();

    // end count time
    double total_computing_2 = MPI_Wtime();

    if (myrank == 0) {
        cout << "query3 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query3 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

void resgen_l4(PSpMat::MPI_DCCols &m_20, PSpMat::MPI_DCCols &m_52, PSpMat::MPI_DCCols &m_42, PSpMat::MPI_DCCols &m_32,
               PSpMat::MPI_DCCols &m_12) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    double t1_start = MPI_Wtime();

    clear_result_time();

    auto commGrid = m_20.getcommgrid();

    // m_20 becoms m_02
    m_20.Transpose();

    vector<IndexType> indl, indr, indj;

    vector<IndexType> order1 = {0, 0, 0, 1, 1, 0}, order2 = {0, 0, 0, 1, 1, 0, 0, 2}, order3 = {0, 0, 0, 1, 1, 0, 0, 2, 0, 3}, order4 = {0, 0, 1, 0, 0, 1, 0, 2, 0, 3, 0, 4};

    double t1_end = MPI_Wtime();
    if (myrank == 0) {
        cout << "begin result generation ......" << endl << flush;
        cout << "\ttranspose matrix and declarations take : " << (t1_end - t1_start) << " s\n" << endl << flush;
    }

    get_local_indices(m_20, indl);
    send_local_indices(commGrid, indl);
    m_20.FreeMemory();

    get_local_indices(m_52, indr);
    send_local_indices(commGrid, indr);
    m_52.FreeMemory();

    local_join(commGrid, indl, indr, 2, 2, 1, 1, order1, indj);
    indl = indj;

    get_local_indices(m_42, indr);
    send_local_indices(commGrid, indr);
    m_42.FreeMemory();

    local_join(commGrid, indl, indr, 3, 2, 1, 1, order2, indj);
    indl = indj;

    get_local_indices(m_32, indr);
    send_local_indices(commGrid, indr);
    m_32.FreeMemory();

    local_join(commGrid, indl, indr, 4, 2, 1, 1, order3, indj);
    indl = indj;

    get_local_indices(m_12, indr);
    send_local_indices(commGrid, indr);
    m_12.FreeMemory();

    local_join(commGrid, indl, indr, 5, 2, 1, 1, order4, indj);
    send_local_results(commGrid, indj.size() / 6);
}

void lubm_l4(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);

    FullyDistVec<IndexType, ElementType> r_20(commWorld, G.getnrow(), 0), l_12(commWorld, G.getnrow(), 0);

    IndexType ind1 = 2808777;
    IndexType ind2 = 291959481;

    if (isPerm) {
        ind1 = nonisov[2808777];
        ind2 = nonisov[291959481];
    }

    r_20.SetElement(ind1, 7);
    l_12.SetElement(ind2, 1);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 4" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(2,0) = G ⊗ {1@(2808777,2808777)}*7" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_20(G);
    multDimApplyPrune(m_20, r_20, Column, true);
    m_20.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : m_(1,2) = G.T() ⊗ m_(2,0).D()*17" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_12(G);
    diagonalizeV(m_20, dm, Row, 17);
    multDimApplyPrune(m_12, dm, Row, true);
    m_12.Transpose();
    m_12.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(1,2) = {1@(291959481,291959481)} × m_(1,2)" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    multDimApplyPrune(m_12, l_12, Row, false);
    m_12.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 4
    if (myrank == 0) {
        cout << "step 4 : m_(3,2) = G.T() ⊗ m_(1,2).T().D()*9" << endl << flush;
    }
    double t4_start = MPI_Wtime();
    auto m_32(G);
    diagonalizeV(m_12, dm, Column, 9);
    multDimApplyPrune(m_32, dm, Row, true);
    m_32.Transpose();
    m_32.PrintInfo();
    double t4_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 4 (Total) : " << (t4_end - t4_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 5
    if (myrank == 0) {
        cout << "step 5 : m_(4,2) = G.T() ⊗ m_(3,2).T().D()*8" << endl << flush;
    }
    double t5_start = MPI_Wtime();
    auto m_42(G);
    diagonalizeV(m_32, dm, Column, 8);
    multDimApplyPrune(m_42, dm, Row, true);
    m_42.Transpose();
    m_42.PrintInfo();
    double t5_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 5 (Total) : " << (t5_end - t5_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 6
    if (myrank == 0) {
        cout << "step 6 : m_(5,2) = G.T() ⊗ m_(4,2).T().D()*2" << endl << flush;
    }
    double t6_start = MPI_Wtime();
    auto m_52(G);
    diagonalizeV(m_42, dm, Column, 2);
    multDimApplyPrune(m_52, dm, Row, true);
    m_52.Transpose();
    m_52.PrintInfo();
    double t6_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 6 (Total) : " << (t6_end - t6_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 7
    if (myrank == 0) {
        cout << "step 7 : m_(2,0) = m_(5,2).T().D() ⊗ m_(2,0)" << endl << flush;
    }
    double t7_start = MPI_Wtime();
    diagonalizeV(m_52, dm, Column);
    multDimApplyPrune(m_20, dm, Row, true);
    m_20.PrintInfo();
    double t7_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 7 (Total) : " << (t7_end - t7_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double query_counting = MPI_Wtime();

    if (myrank == 0) {
        cout << "query4 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query4 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query4 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query4 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query4 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
    resgen_l4(m_20, m_52, m_42, m_32, m_12);
    double resgen_end = MPI_Wtime();

    // end count time
    double total_computing_2 = MPI_Wtime();
    if (myrank == 0) {
        cout << "query4 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query4 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

void resgen_l5(PSpMat::MPI_DCCols &m_20, PSpMat::MPI_DCCols &m_12) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    double t1_start = MPI_Wtime();

    clear_result_time();

    auto commGrid = m_20.getcommgrid();

    // m_20 becoms m_02
    m_20.Transpose();

    vector<IndexType> indl, indr, indj;

    vector<IndexType> order1 = {0, 1, 1, 1, 0, 0};

    double t1_end = MPI_Wtime();
    if (myrank == 0) {
        cout << "begin result generation ......" << endl << flush;
        cout << "\ttranspose matrix and declarations take : " << (t1_end - t1_start) << " s\n" << endl << flush;
    }

    get_local_indices(m_20, indl);
    send_local_indices(commGrid, indl);
    m_20.FreeMemory();

    get_local_indices(m_12, indr);
    send_local_indices(commGrid, indr);
    m_12.FreeMemory();

    local_join(commGrid, indl, indr, 2, 2, 1, 1, order1, indj);
    send_local_results(commGrid, indj.size() / 3);
}

void lubm_l5(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);

    FullyDistVec<IndexType, ElementType> r_20(commWorld, G.getnrow(), 0), l_12(commWorld, G.getnrow(), 0);

    IndexType ind1 = 191176245;
    IndexType ind2 = 2808777;

    if (isPerm) {
        ind1 = nonisov[191176245];
        ind2 = nonisov[2808777];
    }

    r_20.SetElement(ind1, 17);
    l_12.SetElement(ind2, 1);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 5" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(2,0) = G ⊗ {1@(191176245,191176245)}*17" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_20(G);
    multDimApplyPrune(m_20, r_20, Column, true);
    m_20.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : G.T() ⊗ m_(2,0).D()*3" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_12(G);
    diagonalizeV(m_20, dm, Row, 3);
    multDimApplyPrune(m_12, dm, Row, true);
    m_12.Transpose();
    m_12.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(1,2) = {1@(2808777,2808777)} × m_(1,2)" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    multDimApplyPrune(m_12, l_12, Row, false);
    m_12.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 4
    if (myrank == 0) {
        cout << "step 4 : m_(2,0) = m_(1,2).T().D() ⊗ m_(2,0)" << endl << flush;
    }
    double t4_start = MPI_Wtime();
    diagonalizeV(m_12, dm, Column);
    multDimApplyPrune(m_20, dm, Row, true);
    m_20.PrintInfo();
    double t4_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 4 (Total) : " << (t4_end - t4_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double query_counting = MPI_Wtime();

    if (myrank == 0) {
        cout << "query5 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query5 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query5 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query5 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query5 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
    resgen_l5(m_20, m_12);
    double resgen_end = MPI_Wtime();

    // end count time
    double total_computing_2 = MPI_Wtime();
    if (myrank == 0) {
        cout << "query5 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query5 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

void resgen_l6(PSpMat::MPI_DCCols &m_40, PSpMat::MPI_DCCols &m_14, PSpMat::MPI_DCCols &m_34, PSpMat::MPI_DCCols &m_23) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    double t1_start = MPI_Wtime();

    clear_result_time();

    auto commGrid = m_40.getcommgrid();

    // m_40 becoms m_04
    m_40.Transpose();

    vector<IndexType> indl, indr, indj;

    vector<IndexType> order1 = {0, 0, 1, 0, 1, 1}, order2 = {0, 0, 1, 0, 0, 1, 0, 2}, order3 = {0, 0, 1, 0, 0, 1, 0, 2, 0, 3};

    double t1_end = MPI_Wtime();
    if (myrank == 0) {
        cout << "begin result generation ......" << endl << flush;
        cout << "\ttranspose matrix and declarations take : " << (t1_end - t1_start) << " s\n" << endl << flush;
    }

    get_local_indices(m_40, indl);
    send_local_indices(commGrid, indl);
    m_40.FreeMemory();

    get_local_indices(m_34, indr);
    send_local_indices(commGrid, indr);
    m_34.FreeMemory();

    local_join(commGrid, indl, indr, 2, 2, 1, 1, order1, indj);
    local_redistribution(m_23, indj, 3, 1, indl);

    get_local_indices(m_23, indr);
    send_local_indices(commGrid, indr);
    m_23.FreeMemory();

    local_join(commGrid, indl, indr, 3, 2, 1, 1, order2, indj);
    local_redistribution(m_14, indj, 4, 3, indl);

    get_local_indices(m_14, indr);
    send_local_indices(commGrid, indr);
    m_14.FreeMemory();

    local_join(commGrid, indl, indr, 4, 2, 3, 1, order3, indj);
    send_local_results(commGrid, indj.size() / 5);
}

void lubm_l6(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);

    FullyDistVec<IndexType, ElementType> r_40(commWorld, G.getnrow(), 0), l_14(commWorld, G.getnrow(), 0), l_23(
            commWorld, G.getnrow(), 0);

    IndexType ind1 = 130768016;
    IndexType ind2 = 267261320;
    IndexType ind3 = 291959481;

    if (isPerm) {
        ind1 = nonisov[130768016];
        ind2 = nonisov[267261320];
        ind3 = nonisov[291959481];
    }

    r_40.SetElement(ind1, 17);
    l_14.SetElement(ind2, 1);
    l_23.SetElement(ind3, 1);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 6" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(4,0) = G ⊗ {1@(130768016,130768016)}*17" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_40(G);
    multDimApplyPrune(m_40, r_40, Column, true);
    m_40.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : m_(1,4) = G.T() ⊗ m_(4,0).D()*3" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_14(G);
    diagonalizeV(m_40, dm, Row, 3);
    multDimApplyPrune(m_14, dm, Row, true);
    m_14.Transpose();
    m_14.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(1,4) = {1@(267261320,267261320)} × m_(1,4)" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    multDimApplyPrune(m_14, l_14, Row, false);
    m_14.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 4
    if (myrank == 0) {
        cout << "step 4 : m_(3,4) = G ⊗ m_(1,4).T().D()*7" << endl << flush;
    }
    double t4_start = MPI_Wtime();
    auto m_34(G);
    diagonalizeV(m_14, dm, Column, 7);
    multDimApplyPrune(m_34, dm, Column, true);
    m_34.PrintInfo();
    double t4_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 5 (Total) : " << (t4_end - t4_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 5
    if (myrank == 0) {
        cout << "step 5 : m_(2,3) = G.T() ⊗ m_(3,4).D()*17" << endl << flush;
    }
    double t5_start = MPI_Wtime();
    auto m_23(G);
    diagonalizeV(m_34, dm, Row, 17);
    multDimApplyPrune(m_23, dm, Row, true);
    m_23.Transpose();
    m_23.PrintInfo();
    double t5_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t5_end - t5_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 6
    if (myrank == 0) {
        cout << "step 6 : m_(2,3) = {1@(291959481,291959481)} × m_(2,3)" << endl << flush;
    }
    double t6_start = MPI_Wtime();
    multDimApplyPrune(m_23, l_23, Row, false);
    m_23.PrintInfo();
    double t6_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 6 (Total) : " << (t6_end - t6_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 7
    if (myrank == 0) {
        cout << "step 7 : m_(3,4) = m_(2,3).T().D() ⊗ m_(3,4)" << endl << flush;
    }
    double t7_start = MPI_Wtime();
    diagonalizeV(m_23, dm, Column);
    multDimApplyPrune(m_34, dm, Row, true);
    m_34.PrintInfo();
    double t7_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 7 (Total) : " << (t7_end - t7_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 8
    if (myrank == 0) {
        cout << "step 8 : m_(4,0) = m_(3,4).T().D() ⊗ m_(4,0)" << endl << flush;
    }
    double t8_start = MPI_Wtime();
    diagonalizeV(m_34, dm, Column);
    multDimApplyPrune(m_40, dm, Row, true);
    m_40.PrintInfo();
    double t8_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 8 (Total) : " << (t8_end - t8_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double query_counting = MPI_Wtime();

    if (myrank == 0) {
        cout << "query6 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query6 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query6 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query6 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query6 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
    resgen_l6(m_40, m_14, m_34, m_23);
    double resgen_end = MPI_Wtime();

    // end count time
    double total_computing_2 = MPI_Wtime();
    if (myrank == 0) {
        cout << "query6 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query6 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

void resgen_l7(PSpMat::MPI_DCCols &m_52, PSpMat::MPI_DCCols &m_35, PSpMat::MPI_DCCols &m_03, PSpMat::MPI_DCCols &m_43,
                 PSpMat::MPI_DCCols &m_14, PSpMat::MPI_DCCols &m_64) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    double t1_start = MPI_Wtime();

    clear_result_time();

    auto commGrid = m_52.getcommgrid();

    // m_52 becoms m_25
    m_52.Transpose();

    vector<IndexType> indl, indr, indj;

    vector<IndexType> order1 = {0, 0, 1, 0, 1, 1}, order2 = {0, 0, 0, 1, 1, 0, 0, 2}, order3 = {0, 0, 0, 1, 0, 2, 0, 3},
            order4 = {1, 0, 0, 0, 0, 1, 0, 2, 0, 3}, order5 = {1, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4};

    double t1_end = MPI_Wtime();
    if (myrank == 0) {
        cout << "begin result generation ......" << endl << flush;
        cout << "\ttranspose matrix and declarations take : " << (t1_end - t1_start) << " s\n" << endl << flush;
    }

    get_local_indices(m_52, indl);
    send_local_indices(commGrid, indl);
    m_52.FreeMemory();

    get_local_indices(m_35, indr);
    send_local_indices(commGrid, indr);
    m_35.FreeMemory();

    local_join(commGrid, indl, indr, 2, 2, 1, 1, order1, indj);
    send_local_results(commGrid, indj.size() / 3);
    local_redistribution(m_43, indj, 3, 1, indl);

    get_local_indices(m_43, indr);
    send_local_indices(commGrid, indr);
    m_43.FreeMemory();

    local_join(commGrid, indl, indr, 3, 2, 1, 1, order2, indj);
    send_local_results(commGrid, indj.size() / 4);
    local_redistribution(m_64, indj, 4, 2, indl);

    get_local_indices(m_64, indr);
    send_local_indices(commGrid, indr);
    m_64.FreeMemory();

    local_filter(commGrid, indl, indr, 4, 2, 2, 3, 1, 0, order3, indj);
    send_local_results(commGrid, indj.size() / 4);
    indl = indj;

    get_local_indices(m_14, indr);
    send_local_indices(commGrid, indr);
    m_14.FreeMemory();

    local_join(commGrid, indl, indr, 4, 2, 2, 1, order4, indj);
    send_local_results(commGrid, indj.size() / 5);
    local_redistribution(m_03, indj, 5, 2, indl);

    get_local_indices(m_03, indr);
    send_local_indices(commGrid, indr);
    m_03.FreeMemory();

    local_join(commGrid, indl, indr, 5, 2, 2, 1, order5, indj);
    send_local_results(commGrid, indj.size() / 6);
}

void lubm_l7(PSpMat::MPI_DCCols &G, FullyDistVec<IndexType, IndexType> &nonisov, bool isPerm) {
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    clear_query_time();

    auto commWorld = G.getcommgrid();

    FullyDistVec<IndexType, ElementType> dm(commWorld);
    int64_t nnodes = G.getnrow();

    FullyDistVec<IndexType, ElementType> r_52(commWorld, G.getnrow(), 0), l_03(commWorld, G.getnrow(), 0), l_14(
            commWorld, G.getnrow(), 0);

    IndexType ind1 = 291959481;
    IndexType ind2 = 223452631;
    IndexType ind3 = 235928023;

    if (isPerm) {
        ind1 = nonisov[291959481];
        ind2 = nonisov[223452631];
        ind3 = nonisov[235928023];
    }

    r_52.SetElement(ind1, 17);
    l_03.SetElement(ind2, 1);
    l_14.SetElement(ind3, 1);

    // start count time
    double total_computing_1 = MPI_Wtime();

    // ==> step 1
    if (myrank == 0) {
        cout << "\n###############################################################" << endl << flush;
        cout << "Query 7" << endl << flush;
        cout << "###############################################################" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "step 1 : m_(5,2) = G ⊗ {1@(291959481,291959481)}*17" << endl << flush;
    }
    double t1_start = MPI_Wtime();
    auto m_52(G);
    multDimApplyPrune(m_52, r_52, Column, true);
    m_52.PrintInfo();
    double t1_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 1 (Total) : " << (t1_end - t1_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 2
    if (myrank == 0) {
        cout << "step 2 : m_(3,5) = G ⊗ m_(5,2).D()*18" << endl << flush;
    }
    double t2_start = MPI_Wtime();
    auto m_35(G);
    diagonalizeV(m_52, dm, Row, 18);
    multDimApplyPrune(m_35, dm, Column, true);
    m_35.PrintInfo();
    double t2_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 2 (Total) : " << (t2_end - t2_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 3
    if (myrank == 0) {
        cout << "step 3 : m_(0,3) = G.T() ⊗ m_(3,5).D()*17" << endl << flush;
    }
    double t3_start = MPI_Wtime();
    auto m_03(G);
    diagonalizeV(m_35, dm, Row, 17);
    multDimApplyPrune(m_03, dm, Row, true);
    m_03.Transpose();
    m_03.PrintInfo();
    double t3_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 3 (Total) : " << (t3_end - t3_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 4
    if (myrank == 0) {
        cout << "step 4 : m_(0,3) = {1@(223452631,223452631)} × m_(0,3)" << endl << flush;
    }
    double t4_start = MPI_Wtime();
    multDimApplyPrune(m_03, l_03, Row, false);
    m_03.PrintInfo();
    double t4_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 4 (Total) : " << (t4_end - t4_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 5
    if (myrank == 0) {
        cout << "step 5 : m_(4,3) = G.T() ⊗ m_(0,3).T().D()*4" << endl << flush;
    }
    double t5_start = MPI_Wtime();
    auto m_43(G);
    diagonalizeV(m_03, dm, Column, 4);
    multDimApplyPrune(m_43, dm, Row, true);
    m_43.Transpose();
    m_43.PrintInfo();
    double t5_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 5 (Total) : " << (t5_end - t5_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 6
    if (myrank == 0) {
        cout << "step 6 : m_(1,4) = G.T() ⊗ m_(4,3).D()*17" << endl << flush;
    }
    double t6_start = MPI_Wtime();
    auto m_14(G);
    diagonalizeV(m_43, dm, Row, 17);
    multDimApplyPrune(m_14, dm, Row, true);
    m_14.Transpose();
    m_14.PrintInfo();
    double t6_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 6 (Total) : " << (t6_end - t6_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 7
    if (myrank == 0) {
        cout << "step 7 : m_(1,4) = {1@(235928023,235928023)} × m_(1,4)" << endl << flush;
    }
    double t7_start = MPI_Wtime();
    multDimApplyPrune(m_14, l_14, Row, false);
    m_14.PrintInfo();
    double t7_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 7 (Total) : " << (t7_end - t7_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 8
    if (myrank == 0) {
        cout << "step 8 : m_(6,4) = G ⊗ m_(1,4).T().D()*5" << endl << flush;
    }
    double t8_start = MPI_Wtime();
    auto m_64(G);
    diagonalizeV(m_14, dm, Column, 5);
    multDimApplyPrune(m_64, dm, Column, true);
    m_64.PrintInfo();
    double t8_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 8 (Total) : " << (t8_end - t8_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 9
    if (myrank == 0) {
        cout << "step 9 : m_(6,4) = m_(3,5).T().D() × m_(6,4)" << endl << flush;
    }
    double t9_start = MPI_Wtime();
    diagonalizeV(m_35, dm, Column);
    multDimApplyPrune(m_64, dm, Row, false);
    m_64.PrintInfo();
    double t9_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 9 (Total) : " << (t9_end - t9_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 10
    if (myrank == 0) {
        cout << "step 10 : m_(3,5) = m_(3,5) × m_(6,4).D()" << endl << flush;
    }
    double t10_start = MPI_Wtime();
    diagonalizeV(m_64, dm);
    multDimApplyPrune(m_35, dm, Column, false);
    m_35.PrintInfo();
    double t10_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 10 (Total) : " << (t10_end - t10_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 11
    if (myrank == 0) {
        cout << "step 11 : m_(4,3) = m_(6,4).T().D() × m_(4,3)" << endl << flush;
    }
    double t11_start = MPI_Wtime();
    diagonalizeV(m_64, dm, Column);
    multDimApplyPrune(m_43, dm, Row, false);
    m_43.PrintInfo();
    double t11_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 11 (Total) : " << (t11_end - t11_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 12
    if (myrank == 0) {
        cout << "step 12 : m_(3,5) = m_(4,3).T().D() × m_(3,5)" << endl << flush;
    }
    double t12_start = MPI_Wtime();
    diagonalizeV(m_43, dm, Column);
    multDimApplyPrune(m_35, dm, Row, false);
    m_35.PrintInfo();
    double t12_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 12 (Total) : " << (t12_end - t12_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    // ==> step 13
    if (myrank == 0) {
        cout << "step 13 : m_(5,2) = m_(3,5).T().D() × m_(5,2)" << endl << flush;
    }
    double t13_start = MPI_Wtime();
    diagonalizeV(m_35, dm, Column);
    multDimApplyPrune(m_52, dm, Row, false);
    m_52.PrintInfo();
    double t13_end = MPI_Wtime();

    if (myrank == 0) {
        cout << "step 13 (Total) : " << (t13_end - t13_start) << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double query_counting = MPI_Wtime();

    if (myrank == 0) {
        cout << "query7 mmul_scalar time : " << total_mmul_scalar_time << " s" << endl << flush;
        cout << "query7 prune time : " << total_prune_time << " s" << endl << flush;
        cout << "query7 diag_reduce time : " << total_reduce_time << " s" << endl << flush;
        cout << "query7 dim_apply time : " << total_dim_apply_time << " s" << endl << flush;
        cout << "query7 total query execution time : " << query_counting - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }

    double resgen_start = MPI_Wtime();
    resgen_l7(m_52, m_35, m_03, m_43, m_14, m_64);
    double resgen_end = MPI_Wtime();

    // end count time
    double total_computing_2 = MPI_Wtime();
    if (myrank == 0) {
        cout << "query7 result_enum time : " << resgen_end - resgen_start << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
        cout << "query7 time (Total) : " << total_computing_2 - total_computing_1 << " s" << endl << flush;
        cout << "---------------------------------------------------------------" << endl << flush;
    }
}

int main(int argc, char *argv[]) {
    int nprocs, myrank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // initialize comp function array
    initComp();

    if (argc < 2) {
        if (myrank == 0) {
            cout << "Usage: ./magiqScal file isPerm=0" << endl << flush;
        }
        MPI_Finalize();
        return -1;
    }

    {
        // initialization phase
        MPI_Barrier(MPI_COMM_WORLD);
        string Mname(argv[1]);
        
        if (myrank == 0) {
            cout << "###############################################################" << endl << flush;
            cout << "Load Matrix" << endl << flush;
            cout << "###############################################################" << endl << flush;
            cout << "---------------------------------------------------------------" << endl << flush;
            cout << "All procs reading and permuting input graph [" << Mname << "]..." << endl << flush;
        }

        int isPerm = 0;
        if (argc > 2)
            isPerm = atoi(argv[2]);

        PSpMat::MPI_DCCols G(MPI_COMM_WORLD);
        auto commWorld = G.getcommgrid();

        // permute vector 
        FullyDistVec<IndexType, IndexType> nonisov(commWorld);

        double t1 = MPI_Wtime();
        G.ParallelReadMM(Mname, true, selectSecond, isPerm > 0, nonisov);
        double t2 = MPI_Wtime();

        G.PrintInfo();
        float imG = G.LoadImbalance();

        if (myrank == 0) {
            cout << "\tread and permute graph took : " << (t2 - t1) << " s" << endl << flush;
            cout << "\timbalance of G (after random permutation) : " << imG << endl << flush;
        }
        
        double t2_trans = MPI_Wtime();

        if (myrank == 0) {
            cout << "graph load (Total) : " << (t2_trans - t1) << " s" << endl << flush;
            cout << "---------------------------------------------------------------" << endl << flush << flush;;
        }

        // run 7 queries 5 times each
        int total_triall = 5;
        for (int triall = 1; triall <= total_triall; triall++) {
            if(myrank == 0) {cout << "Doing iter " << triall << " of queries..." << endl << flush << "=============================" << endl << flush << flush;}
            lubm_l1(G, nonisov, isPerm > 0);
            lubm_l2(G, nonisov, isPerm > 0);
            lubm_l3(G, nonisov, isPerm > 0);
            lubm_l4(G, nonisov, isPerm > 0);
            lubm_l5(G, nonisov, isPerm > 0);
            lubm_l6(G, nonisov, isPerm > 0);
            lubm_l7(G, nonisov, isPerm > 0);
        }
    }

    MPI_Finalize();
    return 0;
}
