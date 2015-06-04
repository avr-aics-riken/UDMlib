/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file partition_weight.cpp
 * CGNSモデルの重みを持つ分割
 */

#include <mpi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <udmlib.h>

#ifdef __cplusplus
using namespace udm;
#endif

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
    UdmHanler_t model = udm_create_model();

    // index.dfi読込、CGNS読込
    if (udm_load_model(model, argv[1], 0) != UDM_OK) {
        printf("Error : can not load model[index.dfi=%s].\n", argv[1]);
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: loadModel!\n");
    // 出力CGNS構成
    udm_config_setoutputpath(model, "../output_weight");
    // 物理量の設定
    udm_config_setscalarsolution(model, "Weight", Udm_CellCenter, Udm_RealSingle);

    // ゾーンの取得
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        udm_delete_model(model);
        return -1;
    }
    int zone_id = 1;        // 先頭ゾーンとする.

    // 重み設定
    int cell_size = udm_getnum_cells(model, zone_id);
    int n, m;
    float weight = 0.0;
    for (n=1; n<=cell_size; n++) {
        bool boundary_cell = false;
        UdmElementType_t elem_type;
        UdmSize_t node_ids[8];
        int num_nodes = 0;
        udm_get_cellconnectivity(model, zone_id, n, &elem_type, node_ids, &num_nodes);
        float temperature = 0.0;
        for (m=0; m<num_nodes; m++) {
            // 節点（ノード）座標の取得
            UdmReal_t x = 0, y = 0, z = 0;
            udm_get_gridcoordinates(model, zone_id, node_ids[m], &x, &y, &z);
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
        udm_set_partitionweight(model, zone_id, n, weight);
        // デバッグ用 : 要素（セル）に重み物理量の設定
        udm_set_cellsolution_real(model, zone_id, n, "Weight", (UdmReal_t)weight);
    }

    // 分割を行う
    printf("Start :: Zoltan Partition!\n");

    // 分割実行
    ret = udm_partition_zone(model, zone_id);
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("Waring :: partition not change!\n");
    }
    else if (ret != UDM_OK) {
        printf("Error : partition : myrank=%d\n", myrank);
        udm_delete_model(model);
        MPI_Finalize();
        return 0;
    }
    printf("End :: Zoltan Partition!\n");

    // 分割ファイル出力 : step = 0
    printf("Start :: writeModel!\n");
    ret = udm_write_model(model, 0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model.\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel!\n");

    // 再分割実行：重みなし
    printf("Start :: Zoltan Partition : none weight!\n");
    // 重みをクリア
    udm_clear_partitionweight(model, zone_id);
    // 出力CGNS構成
    udm_config_setoutputpath(model, "../output_noneweight");
    ret = udm_partition_zone(model, zone_id);
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("Waring :: partition not change : none weight!\n");
    }
    else if (ret != UDM_OK) {
        printf("Error : partition : myrank=%d  : none weight\n", myrank);
        udm_delete_model(model);
        MPI_Finalize();
        return 0;
    }
    printf("End :: Zoltan Partition : none weight!\n");
    printf("Start :: writeModel : none weight!\n");
    ret = udm_write_model(model, 0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model : none weight.\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel : none weight!\n");

    // 非構造格子モデル破棄
    udm_delete_model(model);
    MPI_Finalize();

    printf("End : partition_weight\n");

    return 0;
}



