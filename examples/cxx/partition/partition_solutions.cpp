/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file partition_solutions.cpp
 * 物理量を持つCGNSモデルの分割
 */

#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <math.h>
#include <mpi.h>
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
    const char usage[] = "usage: mpiexec -np [PARTATION_SIZE] partition_solutions [INPUT_DFI_FILE].";
    const char output_path[] = "../output_solution";
    printf("Start : partition_solutions\n");

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
    model->setOutputPath(output_path);

    // 物理量定義の設定
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (!solutions->existsSolutionConfig("Pressure")) {
        solutions->setSolutionFieldInfo("Pressure", Udm_Vertex, Udm_RealSingle);
        printf("set solution config : Pressure\n");
    }
    if (!solutions->existsSolutionConfig("Velocity")) {
        solutions->setSolutionFieldInfo("Velocity", Udm_Vertex, Udm_RealSingle, Udm_Vector, 3);
        printf("set solution config : Velocity\n");
    }
    if (!solutions->existsSolutionConfig("Temperature")) {
        solutions->setSolutionFieldInfo("Temperature", Udm_CellCenter, Udm_RealSingle);
        printf("set solution config : Temperature\n");
    }

    // ゾーンの取得
    zone = model->getZone();

    // 物理量の設定
    UdmSize_t m, n;
    UdmSize_t node_size = zone->getNumNodes();
    float x, y, z;
    for (n=1; n<=node_size; n++) {
        UdmNode *node = zone->getNode(n);
        node->getCoords(x, y, z);
        float pressure = sqrt(x*x + y*y + z*z);
        float velocity[] = {x, y, z};
        node->setSolutionScalar("Pressure", pressure);
        node->setSolutionVector("Velocity", Udm_RealSingle, velocity, 3);
    }

    UdmSize_t cell_size = zone->getNumCells();
    for (n=1; n<=cell_size; n++) {
        UdmCell *cell = zone->getCell(n);
        node_size = cell->getNumNodes();
        float temperature = 0.0;
        for (m=1; m<=node_size; m++) {
            UdmNode *node = cell->getNode(m);
            node->getCoords(x, y, z);
            temperature += sqrt(x*x + z*z);
        }
        temperature = temperature/node_size;
        cell->setSolutionScalar("Temperature", temperature);
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
    printf("End :: writeModel! [output path = %s]\n", output_path);

    // 非構造格子モデル破棄
    delete model;
    MPI_Finalize();

    printf("End : partition_solutions\n");

    return 0;
}



