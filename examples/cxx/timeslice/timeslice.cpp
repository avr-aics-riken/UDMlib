/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file timeslice.cpp
 * 時系列CGNSファイルの出力
 */

#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <getopt.h>
#include <sys/stat.h>
#include <udmlib.h>
#include <model/UdmModel.h>

using namespace std;
using namespace udm;

#define  MAX_TIMESTEP           100
#define  WRITE_STEP              10
#define  TIMESTEP_TIME           0.01


/**
 * ヘルプ表示
 */
void print_help()
{
    const char usage[] = "usage: timeslice [INDEX_DFI] OPTIONS.";
    printf("%s\n", usage);
    printf("OPTIONS:\n");
    printf("    --output=[OUTPUT_PATH]   出力パス（デフォルト=\"output\"） \n");
    printf("    --without_cell           要素（セル）のCGNS:FlowSolutionを出力しません。(デフォルト=出力する) \n");
    printf("    --without_node           節点（ノード）のCGNS:FlowSolutionを出力しません。(デフォルト=出力する) \n");
    printf("    -h --help                                     ヘルプ出力\n");
    printf("(例)\n");
    printf("    timeslice input/index.dfi --without_cell \n");
    printf("\n");
    return;
}

/**
 * 起動オプションを取得する.
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @param filename        入力DFIファイル名
 * @param output_path          出力パス（デフォルト=output)
 * @param without_cell     要素（セル）のCGNS:FlowSolutionを出力しない.
 * @param without_node     節点（ノード）のCGNS:FlowSolutionを出力しない.
 * @return                取得成功
 */
bool get_options(int argc, char *argv[],
                char *filename,
                char *output_path,
                bool &without_cell,
                bool &without_node)
{
    struct option long_options[] = {
        {"output", required_argument,        NULL, 'o'},
        {"without_cell", no_argument,        NULL, 'c'},
        {"without_node", no_argument,        NULL, 'n'},
        {"help",  no_argument,  NULL, 'h'},
        {0, 0, 0, 0}
    };

    bool help = false;
    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "ocn", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'o':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(output_path, optarg);
            break;
        case 'c':        // without_cell
            without_cell = true;
            break;
        case 'n':        // without_node
            without_node = true;
            break;
        case 'h':
            help = true;
            break;
        // 解析できないオプションが見つかった場合は「?」
        case '?':
            help = true;
            break;
        }
    }
    if (optind >= 0 && optind < argc) {
        strcpy(filename, argv[optind]);
    }

    return !help;
}

/**
 * 計算を行う.
 * @param model        モデル
 */
void initialize_solver(UdmModel *model)
{
    UdmSize_t n, i;
    float x, y, z;
    UdmZone *zone = model->getZone();
    float value[3] = {0.0, 0.0, 0.0};
    UdmDfiConfig* config = model->getDfiConfig();
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();

    for (n=1; n<=zone->getNumNodes(); n++) {
        UdmNode *node = zone->getNode(n);
        node->getCoords(x, y, z);
        if (x == 0.0 && z == 0.0) {
            if (solutions->existsSolutionConfig("Pressure")) {
                node->setSolutionScalar("Pressure", 100.0);
                node->setSolutionScalar("p0", 0.0);
            }
            if (solutions->existsSolutionConfig("Motion")) {
                node->setSolutionVector("Motion", value);
            }
            if (solutions->existsSolutionConfig("Material")) {
                node->setSolutionScalar("Material", 1);
            }
            if (solutions->existsSolutionConfig("Temperature")) {
                UdmSize_t num_cells = node->getNumConnectivityCells();
                for (i=1; i<=num_cells; i++) {
                    UdmCell *cell = node->getConnectivityCell(i);
                    cell->setSolutionScalar("Temperature", 100.0);
                    cell->setSolutionScalar("t0", 100.0);
                }
            }
        }
    }
    return;
}


/**
 * 計算を行う.
 * @param model        モデル
 * @param loop        ステップ数
 */
void calculate_solver(UdmModel *model, int loop)
{
    UdmSize_t i, n;
    float x0, y0, z0;
    float pressure, p0;
    float temperature;
    float sum, max, tmp;
    int count;
    UdmZone *zone = model->getZone();
    UdmDfiConfig* config = model->getDfiConfig();
    UdmFileInfoConfig* fileinfo = config->getFileinfoConfig();
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    UdmSize_t node_id = 0;
    int rankno = 0;

    for (n=1; n<=zone->getNumNodes(); n++) {
        UdmNode *node = zone->getNode(n);
        node_id = node->getId();
        rankno = model->getMpiRankno();
        node->getCoords(x0, y0, z0);

        if (solutions->existsSolutionConfig("Pressure")) {
            // 物理量の取得
            p0 = pressure = 0.0;        // Pressureが未設定の場合は、値は返ってこない
            node->getSolutionScalar("Pressure", pressure);
            node->getSolutionScalar("p0", p0);

            // 接続節点（ノード）から物理量の計算
            sum = 0.0;
            count = 0;
            UdmSize_t num_neighbor = node->getNumNeighborNodes();
            for (i=1; i<=num_neighbor; i++) {
                UdmNode *neighbor_node = node->getNeighborNode(i);
                tmp = 0.0;
                neighbor_node->getSolutionScalar("p0", tmp);
                sum += tmp;
            }

            // 物理量:Pressureの設定
            float dp = (sum/num_neighbor)*0.1;
            if (pressure + dp <= 100.0) pressure += dp;
            else pressure = 100.0;
            node->setSolutionScalar("Pressure", pressure);
        }

        float motion_z0 = sin((float)(loop-1)/(float)MAX_TIMESTEP*2.0*M_PI)*x0/2.0;
        float motion_z1 = sin((float)loop/(float)MAX_TIMESTEP*2.0*M_PI)*x0/2.0;
        if (solutions->existsSolutionConfig("Motion")) {
            // 物理量:Motionの設定
            float motions[3] = {0.0, 0.0, 0.0};
            node->getSolutionVector("Motion", motions);
            motions[2] = motion_z1;
            node->setSolutionVector("Motion", motions);
        }
        // 変位座標値の設定
        if (!fileinfo->isFileGridConstant()) {
            node->setCoords(x0, y0, z0+(motion_z1-motion_z0));
        }
    }

    if (solutions->existsSolutionConfig("Pressure")) {
        for (n=1; n<=zone->getNumNodes(); n++) {
            UdmNode *node = zone->getNode(n);
            pressure = 0.0;
            node->getSolutionScalar("Pressure", pressure);
            node->setSolutionScalar("p0", pressure);
        }
    }

    if (solutions->existsSolutionConfig("Temperature")) {
        for (n=1; n<=zone->getNumCells(); n++) {
            UdmCell *cell = zone->getCell(n);
            temperature = 0.0;
            cell->getSolutionScalar("Temperature", temperature);
            float t0 = 0;
            cell->getSolutionScalar("t0", t0);

            // 隣接要素（セル）から物理量の計算
            max = 0.0;
            UdmSize_t num_neighbor = cell->getNumNeighborCells();
            for (i=1; i<=num_neighbor; i++) {
                UdmCell *neighbor_cell = cell->getNeighborCell(i);
                float neighbor_t0 = 0.0;
                neighbor_cell->getSolutionScalar("t0", neighbor_t0);
                if (max < neighbor_t0) max = neighbor_t0;
            }
            cell->setSolutionScalar("t0", temperature);
            cell->setSolutionScalar("Temperature", t0 + (max - t0)*0.1);
        }
    }

    return;
}


/**
 * メイン関数
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @return                終了コード
 */
int main(int argc, char *argv[]) {
    UdmError_t ret;
    int myrank, num_procs;
    char dfiname[128] = {0x00};
    char output_path[128] = {"output"};
    bool without_cell = false;
    bool without_node = false;

    // 起動引数の取得
    if (!get_options(argc, argv, dfiname, output_path, without_cell, without_node)) {
        print_help();
        return 1;
    }
    if (strlen(dfiname) == 0) {
        printf("Error : please input index.dfi\n");
        print_help();
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (myrank == 0) {
        printf("[rankno=%d] Start : timeslice\n", myrank);
        // 読込DFIファイル名
        printf("[rankno=%d] Start :: loadModel [DFI FILE = %s]!\n", myrank, dfiname);
    }

    // モデルの生成
    UdmModel *model = new UdmModel();

    if (model->loadModel(dfiname) != UDM_OK) {
        printf("[rankno=%d] Error : can not load model[index.dfi=%s].\n", myrank, dfiname);
        delete model;
        MPI_Finalize();
        return -1;
    }
    printf("[rankno=%d] End :: loadModel!\n", myrank);

    // 物理量設定
    UdmDfiConfig* config = model->getDfiConfig();
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    if (!without_node) {
        solutions->setSolutionFieldInfo("Pressure", Udm_Vertex, Udm_RealSingle);
        solutions->setSolutionFieldInfo("p0", Udm_Vertex, Udm_RealSingle);
        solutions->setSolutionFieldInfo("Motion", Udm_Vertex, Udm_RealSingle, Udm_Vector, 3);
        solutions->setSolutionFieldInfo("Material", Udm_Vertex, Udm_Integer, Udm_Scalar, 1, true);
    }
    else {
        solutions->removeSolutionConfig("Pressure");
        solutions->removeSolutionConfig("p0");
        solutions->removeSolutionConfig("Motion");
        solutions->removeSolutionConfig("Material");
    }
    if (!without_cell) {
        solutions->setSolutionFieldInfo("Temperature", Udm_CellCenter, Udm_RealSingle);
        solutions->setSolutionFieldInfo("t0", Udm_CellCenter, Udm_RealSingle);
    }
    else {
        solutions->removeSolutionConfig("Temperature");
        solutions->removeSolutionConfig("t0");
    }

    // ソルバ：初期化
    initialize_solver(model);

    // 出力パス設定
    model->setOutputPath(output_path);

    // 分割実行
    printf("[rankno=%d] Start :: Zoltan Partition!\n", myrank);
    ret = model->partitionZone();
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("[rankno=%d] Waring :: partition not change!\n", myrank);
    }
    else if (ret != UDM_OK) {
        printf("[rankno=%d] Error : partition!\n", myrank);
        delete model;
        MPI_Finalize();
        return 0;
    }
    printf("[rankno=%d] End :: Zoltan Partition!\n", myrank);
    MPI_Barrier(MPI_COMM_WORLD);

#ifdef _DEBUG
    if (num_procs > 1) {
        // 内部境界出力
        char rank_filename[128] = {0x00};
        sprintf(rank_filename, "output/rank_connectivity_id%08d.cgns", myrank);
        ret = model->writeRankConnectivity(rank_filename);
        if (ret != UDM_OK) {
            printf("[for debug] Error : can not writeRankConnectivity[write_filename=%s]\n.", rank_filename);
        }

        // 仮想セル出力
        char virtual_filename[128] = {0x00};
        sprintf(virtual_filename, "output/virtual_cells_id%08d.cgns", myrank);
        ret = model->writeVirtualCells(virtual_filename);
        if (ret != UDM_OK) {
            printf("Error : can not writeVirtualCells[write_filename=%s].\n", virtual_filename);
        }
    }
#endif

    // time iterator
    int loop = 0;
    float time = 0;

    // 分割ファイル出力 : step = 0
    ret = model->writeModel(loop, time);
    if (myrank == 0) printf("[step=%d] writeModel!\n", loop);
    if (ret != UDM_OK) {
        printf("[rankno=%d] Error : can not write model.\n",  myrank);
        delete model;
        MPI_Finalize();
        return -1;
    }

    for (loop = 1; loop <= MAX_TIMESTEP; loop++, time+=TIMESTEP_TIME) {
        if (myrank == 0) printf("[step=%d] start!\n", loop);

        // ソルバ：ステップ計算
        calculate_solver(model, loop);

        // 仮想セルの転送
        model->transferVirtualCells();

        if (loop%WRITE_STEP == 0) {
            if (myrank == 0) printf("[step=%d] writeModel!\n", loop);
            // 分割ファイル出力 : step = 0
            ret = model->writeModel(loop, time);
            if (ret != UDM_OK) {
                printf("[rankno=%d] Error : can not write model.\n",  myrank);
                delete model;
                MPI_Finalize();
                return -1;
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 非構造格子モデル破棄
    delete model;
    MPI_Finalize();

    printf("[rankno=%d] End : timeslice\n", myrank);

    return 0;
}



