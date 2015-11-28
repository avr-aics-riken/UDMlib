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
 * @file timeslice.cpp
 * 時系列CGNSファイルの出力
 */

#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <sys/stat.h>
#include <udmlib.h>

#ifdef __cplusplus
using namespace udm;
#endif

#define  MAX_TIMESTEP           100
#define  WRITE_STEP             10
#define  TIMESTEP_TIME          0.01


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
                bool *without_cell,
                bool *without_node)
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
            *without_cell = true;
            break;
        case 'n':        // without_node
            *without_node = true;
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
void initialize_solver(UdmHanler_t model)
{
    UdmSize_t n, i;
    UdmReal_t x, y, z;
    UdmReal_t value[3] = {0.0, 0.0, 0.0};
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        return;
    }
    int zone_id = 1;
    if (udm_getnum_sections(model, zone_id) <= 0) {
        printf("Error : can not get section.\n");
        return;
    }
    int element_id = 1;

    UdmSize_t num_nodes = udm_getnum_nodes(model, zone_id);
    for (n=1; n<=num_nodes; n++) {
        udm_get_gridcoordinates(model, zone_id, n, &x, &y, &z);
        if (x == 0.0 && z == 0.0) {
            if (udm_config_existssolution(model, "Pressure")) {
                udm_set_nodesolution_real(model, zone_id, n, "Pressure", 100.0);
                udm_set_nodesolution_real(model, zone_id, n, "p0", 0.0);
            }
            if (udm_config_existssolution(model, "Motion")) {
                udm_set_nodesolutions_real(model, zone_id, n, "Motion", value, 3);
            }
            if (udm_config_existssolution(model, "Material")) {
                udm_set_nodesolution_integer(model, zone_id, n, "Material", 1);
            }
            if (udm_config_existssolution(model, "Temperature")) {
                int num_cells = udm_getnum_nodeconnectivity(model, zone_id, n);
                UdmSize_t *cell_ids = (UdmSize_t*)malloc(num_cells*sizeof(UdmSize_t));
                UdmRealityType_t *neighbor_types = (UdmRealityType_t*)malloc(num_cells*sizeof(UdmRealityType_t));
                udm_get_nodeconnectivity(model, zone_id, n, cell_ids, neighbor_types, &num_cells);
                for (i=0; i<num_cells; i++) {
                    if (neighbor_types[i] == Udm_Virtual) continue;
                    udm_set_cellsolution_real(model, zone_id, cell_ids[i], "Temperature", 100.0);
                    udm_set_cellsolution_real(model, zone_id, cell_ids[i], "t0", 100.0);
                }
                free(cell_ids);
                free(neighbor_types);
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
void calculate_solver(UdmHanler_t model, int loop)
{
    UdmSize_t i, n;
    UdmReal_t x0, y0, z0;
    UdmReal_t pressure, p0;
    UdmReal_t temperature;
    UdmReal_t sum, max, tmp;
    int count;
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        return;
    }
    int zone_id = 1;
    if (udm_getnum_sections(model, zone_id) <= 0) {
        printf("Error : can not get section.\n");
        return;
    }
    int element_id = 1;

    UdmSize_t num_nodes = udm_getnum_nodes(model, zone_id);
    for (n=1; n<=num_nodes; n++) {
        udm_get_gridcoordinates(model, zone_id, n, &x0, &y0, &z0);
        if (udm_config_existssolution(model, "Pressure")) {
            // 物理量の取得
            p0 = pressure = 0.0;        // Pressureが未設定の場合は、値は返ってこない
            udm_get_nodesolution_real(model, zone_id, n, "Pressure", &pressure);
            udm_get_nodesolution_real(model, zone_id, n, "p0", &p0);

            // 接続節点（ノード）から物理量の計算
            sum = 0.0;
            count = 0;
            int num_neighbor = udm_getnum_neighbornodes(model, zone_id, n);
            UdmSize_t *neighbor_nodeids = (UdmSize_t*)malloc(sizeof(UdmSize_t)*num_neighbor);
            UdmRealityType_t *neighbor_types = (UdmRealityType_t*)malloc(sizeof(UdmRealityType_t)*num_neighbor);
            udm_get_neighbornodes(model, zone_id, n, neighbor_nodeids, neighbor_types, &num_neighbor);
            for (i=0; i<num_neighbor; i++) {
                tmp = 0.0;
                udm_get_nodesolution_real(model, zone_id, neighbor_nodeids[i], "p0", &tmp);
                sum += tmp;
            }
            free(neighbor_nodeids);
            free(neighbor_types);

            // 物理量:Pressureの設定
            UdmReal_t dp = (sum/num_neighbor)*0.1;
            if (pressure + dp <= 100.0) pressure += dp;
            else pressure = 100.0;
            udm_set_nodesolution_real(model, zone_id, n, "Pressure", pressure);
        }

        UdmReal_t motion_z0 = sin((float)(loop-1)/(float)MAX_TIMESTEP*2.0*M_PI)*x0/2.0;
        UdmReal_t motion_z1 = sin((float)loop/(float)MAX_TIMESTEP*2.0*M_PI)*x0/2.0;
        if (udm_config_existssolution(model, "Motion")) {
            // 物理量:Motionの設定
            UdmReal_t motions[3] = {0.0, 0.0, 0.0};
            int size = 0;
            udm_get_nodesolutions_real(model, zone_id, n, "Motion", motions, &size);
            motions[2] = motion_z1;
            udm_set_nodesolutions_real(model, zone_id, n, "Motion", motions, 3);
        }
        // 変位座標値の設定
        if (!udm_config_isfilegridconstant(model)) {
            udm_set_gridcoordinates(model, zone_id, n, x0, y0, z0+(motion_z1-motion_z0));
        }
    }

    if (udm_config_existssolution(model, "Pressure")) {
        for (n=1; n<=num_nodes; n++) {
            pressure = 0.0;
            udm_get_nodesolution_real(model, zone_id, n, "Pressure", &pressure);
            udm_set_nodesolution_real(model, zone_id, n, "p0", pressure);
        }
    }

    if (udm_config_existssolution(model, "Temperature")) {
        UdmSize_t num_cells = udm_getnum_cells(model, zone_id);
        for (n=1; n<=num_cells; n++) {
            temperature = 0.0;
            int size = 0;
            udm_get_cellsolution_real(model, zone_id, n, "Temperature", &temperature);
            UdmReal_t t0 = 0;
            udm_get_cellsolution_real(model, zone_id, n, "t0", &t0);

            // 隣接要素（セル）から物理量の計算
            max = 0.0;
            UdmSize_t num_neighbor = udm_getnum_neighborcells(model, zone_id, n);
            UdmSize_t *neighbor_cellids = (UdmSize_t*)malloc(sizeof(UdmSize_t)*num_neighbor);
            UdmRealityType_t *neighbor_types = (UdmRealityType_t*)malloc(sizeof(UdmRealityType_t)*num_neighbor);
            udm_get_neighborcells(model, zone_id, n, neighbor_cellids, neighbor_types, &size);
            for (i=0; i<num_neighbor; i++) {
                UdmReal_t neighbor_t0 = 0.0;
                udm_get_cellsolution_real(model, zone_id, neighbor_cellids[i], "t0", &neighbor_t0);
                if (max < neighbor_t0) max = neighbor_t0;
            }
            free(neighbor_cellids);
            free(neighbor_types);

            udm_set_cellsolution_real(model, zone_id, n, "t0", temperature);
            udm_set_cellsolution_real(model, zone_id, n, "Temperature", t0 + (max - t0)*0.1);
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
    if (!get_options(argc, argv, dfiname, output_path, &without_cell, &without_node)) {
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
    UdmHanler_t model = udm_create_model();
    if (udm_load_model(model, dfiname, 0) != UDM_OK) {
        printf("[rankno=%d] Error : can not load model[index.dfi=%s].\n", myrank, dfiname);
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("[rankno=%d] End :: loadModel!\n", myrank);

    // 物理量設定
    if (!without_node) {
        udm_config_setscalarsolution(model, "Pressure", Udm_Vertex, Udm_RealSingle);
        udm_config_setscalarsolution(model, "p0", Udm_Vertex, Udm_RealSingle);
        udm_config_setsolution(model, "Motion", Udm_Vertex, Udm_RealSingle, Udm_Vector, 3, false);
        udm_config_setsolution(model, "Material", Udm_Vertex, Udm_Integer, Udm_Scalar, 1, true);
    }
    else {
        udm_config_removesolution(model, "Pressure");
        udm_config_removesolution(model, "p0");
        udm_config_removesolution(model, "Motion");
        udm_config_removesolution(model, "Material");
    }
    if (!without_cell) {
        udm_config_setscalarsolution(model, "Temperature", Udm_CellCenter, Udm_RealSingle);
        udm_config_setscalarsolution(model, "t0", Udm_CellCenter, Udm_RealSingle);
    }
    else {
        udm_config_removesolution(model, "Temperature");
        udm_config_removesolution(model, "t0");
    }

    // ソルバ：初期化
    initialize_solver(model);

    // 出力パス設定
    udm_config_setoutputpath(model, output_path);

    // 分割実行
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        udm_delete_model(model);
        return -1;
    }
    int zone_id = 1;
    printf("[rankno=%d] Start :: Zoltan Partition!\n", myrank);
    ret = udm_partition_zone(model, zone_id);
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("[rankno=%d] Waring :: partition not change!\n", myrank);
    }
    else if (ret != UDM_OK) {
        printf("[rankno=%d] Error : partition!\n", myrank);
        udm_delete_model(model);
        MPI_Finalize();
        return 0;
    }
    printf("[rankno=%d] End :: Zoltan Partition!\n", myrank);
    MPI_Barrier(MPI_COMM_WORLD);

    // time iterator
    int loop = 0;
    float time = 0;

    // 分割ファイル出力 : step = 0
    ret = udm_write_model(model, loop, time);
    if (myrank == 0) printf("[step=%d] writeModel!\n", loop);
    if (ret != UDM_OK) {
        printf("[rankno=%d] Error : can not write model.\n",  myrank);
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }

    for (loop = 1; loop <= MAX_TIMESTEP; loop++, time+=TIMESTEP_TIME) {
        if (myrank == 0) printf("[step=%d] start!\n", loop);

        // ソルバ：ステップ計算
        calculate_solver(model, loop);

        // 仮想セルの転送
        udm_transfer_virtualcells(model);

        if (loop%WRITE_STEP == 0) {
            if (myrank == 0) printf("[step=%d] writeModel!\n", loop);
            // 分割ファイル出力 : step = 0
            ret = udm_write_model(model, loop, time);
            if (ret != UDM_OK) {
                printf("[rankno=%d] Error : can not write model.\n",  myrank);
                udm_delete_model(model);
                MPI_Finalize();
                return -1;
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 非構造格子モデル破棄
    udm_delete_model(model);
    MPI_Finalize();

    printf("[rankno=%d] End : timeslice\n", myrank);

    return 0;
}



