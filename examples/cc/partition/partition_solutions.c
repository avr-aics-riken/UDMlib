/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file partition_solutions.c
 * 物理量を持つCGNSモデルの分割
 */

#include <mpi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
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
    udm_config_setoutputpath(model, output_path);

    // 物理量定義の設定
    if (!udm_config_existssolution(model, "Pressure")) {
        udm_config_setscalarsolution(model, "Pressure", Udm_Vertex, Udm_RealSingle);
        printf("set solution config : Pressure\n");
    }
    if (!udm_config_existssolution(model, "Velocity")) {
        udm_config_setsolution(model, "Velocity", Udm_Vertex, Udm_RealSingle, Udm_Vector, 3, false);
        printf("set solution config : Velocity\n");
    }
    if (!udm_config_existssolution(model, "Temperature")) {
        udm_config_setscalarsolution(model, "Temperature", Udm_CellCenter, Udm_RealSingle);
        printf("set solution config : Temperature\n");
    }

    // ゾーンの取得
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        udm_delete_model(model);
        return -1;
    }
    int zone_id = 1;        // 先頭ゾーンとする.

    // 物理量の設定
    UdmSize_t m, n;
    UdmSize_t node_size = udm_getnum_nodes(model, zone_id);
    UdmReal_t x, y, z;
    for (n=1; n<=node_size; n++) {
        udm_get_gridcoordinates(model, zone_id, n, &x, &y, &z);
        UdmReal_t pressure = sqrt(x*x + y*y + z*z);
        UdmReal_t velocity[] = {x, y, z};
        udm_set_nodesolution_real(model, zone_id, n, "Pressure", pressure);
        udm_set_nodesolutions_real(model, zone_id, n, "Velocity", velocity, 3);
    }

    UdmSize_t cell_size = udm_getnum_cells(model, zone_id);
    for (n=1; n<=cell_size; n++) {
        UdmElementType_t elem_type;
        UdmSize_t node_ids[8];
        int num_nodes;
        udm_get_cellconnectivity(model, zone_id, n, &elem_type, node_ids, &num_nodes);
        UdmReal_t temperature = 0.0;
        for (m=0; m<num_nodes; m++) {
            udm_get_gridcoordinates(model, zone_id, node_ids[m], &x, &y, &z);
            temperature += sqrt(x*x + z*z);
        }
        temperature = temperature/node_size;
        udm_set_cellsolution_real(model, zone_id, n, "Temperature", temperature);
    }

    // 分割を行う
    printf("Start :: Zoltan Partition!\n");
    // 分割パラメータ=グラフ選択 : PARTITION
    // udm_partition_setgraph(model, "PARTITION");
    // 分割パラメータ=ハイパーグラフ選択 : PARTITION
    udm_partition_sethypergraph(model, "PARTITION");
    // Zoltanデバッグレベルを1に設定
    udm_partition_setdebuglevel(model, 1);

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
    printf("End :: writeModel! [output path = %s]\n", output_path);

    // 非構造格子モデル破棄
    udm_delete_model(model);
    MPI_Finalize();

    printf("End : partition_solutions\n");

    return 0;
}



