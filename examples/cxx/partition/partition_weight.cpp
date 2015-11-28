// ##################################################################################
//
// UDMlib - Unstructured Data Management Library
//
// Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
// All rights reserved.
//
// Copyright (c) 2015 Advanced Institute for Computational Science, RIKEN.
// All rights reserved.
//
// ###################################################################################

/**
 * @file partition_weight.cpp
 * CGNSモデルの重みを持つ分割
 */

#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <udmlib.h>
#include <model/UdmModel.h>

using namespace std;
using namespace udm;


/**
 * メイン関数
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @return                終了コード
 */
int main(int argc, char *argv[]) {
    UdmError_t ret;
    int myrank, num_procs;
    const char usage[] = "usage: mpiexec -np [PARTATION_SIZE] partition_weight [INPUT_DFI_FILE].";

    printf("Start : partition_weight\n");

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc <= 1) {
        printf("Error : Please input.dfi file.\n");
        printf("%s\n", usage);
        MPI_Finalize();
        return 1;
    }

    printf("Start :: loadModel!\n");
    // モデルの生成
    UdmModel *model = new UdmModel();
    UdmZone *zone = NULL;
    UdmDfiConfig* config = model->getDfiConfig();

    // index.dfi読込、CGNS読込
    if (model->loadModel(argv[1]) != UDM_OK) {
        printf("Error : can not load model[index.dfi=%s].\n", argv[1]);
        delete model;
        MPI_Finalize();
        return -1;
    }
    printf("End :: loadModel!\n");
    // 出力CGNS構成
    config->setOutputPath("../output_weight");
    // 物理量の設定
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    solutions->setSolutionFieldInfo("Weight", Udm_CellCenter, Udm_RealSingle);

    // ゾーンの取得
    zone = model->getZone();

    // 重み設定
    int cell_size = zone->getNumCells();
    int n;
    float weight = 0.0;
    for (n=1; n<=cell_size; n++) {
        UdmCell *cell = zone->getCell(n);
        int node_size = cell->getNumNodes();
        int m;
        bool boundary_cell = false;
        for (m=1; m<=node_size; m++) {
            // 節点（ノード）座標の取得
            UdmNode *node = cell->getNode(m);
            float x = 0, y = 0, z = 0;
            node->getCoords(x, y, z);
            // -J面に重み設定を行う
            if (y == 0.0) {
                // 重み設定面
                boundary_cell = true;
                break;
            }
        }
        if (boundary_cell) {
            weight = 10.0;
        }
        else {
            weight = 1.0;
        }
        // 要素（セル）に重み設定
        cell->setPartitionWeight(weight);
        // デバッグ用 : 要素（セル）に重み物理量の設定
        cell->setSolutionScalar("Weight", weight);
    }

    // 分割を行う
    printf("Start :: Zoltan Partition!\n");
    UdmLoadBalance *partition = model->getLoadBalance();

    // 分割パラメータ=グラフ選択 : PARTITION
    // partition->setGraphParameters("PARTITION");
    // 分割パラメータ=ハイパーグラフ選択 : PARTITION
    partition->setHyperGraphParameters("PARTITION");
    // Zoltanデバッグレベルを1に設定
    partition->setZoltanDebugLevel(1);

    // 分割実行
    ret = model->partitionZone();
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("Waring :: partition not change!\n");
    }
    else if (ret != UDM_OK) {
        printf("Error : partition : myrank=%d\n", myrank);
        delete model;
        MPI_Finalize();
        return 0;
    }
    printf("End :: Zoltan Partition!\n");

    // 分割ファイル出力 : step = 0
    printf("Start :: writeModel!\n");
    ret = model->writeModel(0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model.\n");
        delete model;
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel!\n");

    // 再分割実行：重みなし
    printf("Start :: Zoltan Partition : none weight!\n");
    // 重みをクリア
    zone->clearPartitionWeight();
    // 出力CGNS構成
    config->setOutputPath("../output_noneweight");
    ret = partition->partitionZone(zone);
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("Waring :: partition not change : none weight!\n");
    }
    else if (ret != UDM_OK) {
        printf("Error : partition : myrank=%d  : none weight\n", myrank);
        delete model;
        MPI_Finalize();
        return 0;
    }
    printf("End :: Zoltan Partition : none weight!\n");
    printf("Start :: writeModel : none weight!\n");
    ret = model->writeModel(0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model : none weight.\n");
        delete model;
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel : none weight!\n");

    // 非構造格子モデル破棄
    delete model;
    MPI_Finalize();

    printf("End : partition_weight\n");

    return 0;
}



