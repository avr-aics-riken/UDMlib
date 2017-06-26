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
 * @file partition.c
 * CGNSモデルの分割
 */

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/stat.h>
#include "udmlib.h"

#ifdef __cplusplus
using namespace udm;
#endif

/**
 * ヘルプ表示
 */
void print_help()
{
    const char usage[] = "usage: partition [INDEX_DFI] OPTIONS.";
    printf("%s\n", usage);
    printf("OPTIONS:\n");
    printf("    --output=[OUTPUT_PATH]   出力パス（デフォルト=\"../output\"） \n");
    printf("    --enable_hyper           Zoltan::PACKAGE=HYPERGRAPH (デフォルト) \n");
    printf("    --enable_graph           Zoltan::PACKAGE=GRAPH \n");
    printf("    --enable_partition       Zoltan::LB_APPROACH=PARTITION (デフォルト) \n");
    printf("    --enable_repartition     Zoltan::LB_APPROACH=REPARTITION \n");
    printf("    --step=[STEP_NO]         ロードを行う時系列ステップ番号を指定します。\n");
    printf("    --with-mpirank-path=[MPIRANK_PATH]    MPIランク番号のディレクトリ毎にindex.dfiを配置します。 \n");
    printf("                                          INDEX_DFIは無視されます。 \n");
    printf("    -h --help                                     ヘルプ出力\n");
    printf("(例)\n");
    printf("    partition input/index.dfi --enable_hyper --enable_repartition \n");
    printf("\n");
    return;
}


/**
 * 起動オプションを取得する.
 * @param argc                 起動引数の数
 * @param argv                 起動引数の文字列
 * @param filename             入力DFIファイル名
 * @param output_path          出力パス（デフォルト=../output)
 * @param enable_hyper         Zoltan::PACKAGE=HYPERGRAPH (デフォルト)
 * @param enable_graph         Zoltan::PACKAGE=GRAPH
 * @param enable_partition     Zoltan::LB_APPROACH=PARTITION (デフォルト)
 * @param enable_repartition   Zoltan::LB_APPROACH=REPARTITION
 * @param stepno               ロード時系列ステップ番号
 * @param mpirank_path         MPIランク番号のディレクトリ
 * @return                取得成功
 */
bool get_options(int argc, char *argv[],
                char *filename,
                char *output_path,
                bool *enable_hyper,
                bool *enable_graph,
                bool *enable_partition,
                bool *enable_repartition,
                int *stepno,
                char *mpirank_path)
{
    struct option long_options[] = {
        {"output", required_argument,        NULL, 'o'},
        {"enable_hyper", no_argument,        NULL, 'y'},
        {"enable_graph", no_argument,        NULL, 'g'},
        {"enable_partition", no_argument,        NULL, 'p'},
        {"enable_repartition", no_argument,        NULL, 'r'},
        {"step", required_argument,   NULL, 's'},
        {"with-mpirank-path", required_argument,   NULL, 'm'},
        {"help",  no_argument,  NULL, 'h'},
        {0, 0, 0, 0}
    };

    bool help = false;
    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "oygprm:", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'o':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(output_path, optarg);
            break;
        case 'y':        // enable_hyper
            *enable_hyper = true;
            *enable_graph = false;
            break;
        case 'g':        // enable_graph
            *enable_hyper = false;
            *enable_graph = true;
            break;
        case 'p':        // enable_partition
            *enable_partition = true;
            *enable_repartition = false;
            break;
        case 'r':        // enable_repartition
            *enable_partition = false;
            *enable_repartition = true;
            break;
        case 's':        // step
            *stepno = atoi(optarg);
            if (stepno < 0) {
                printf("Error : stepno less than zero.");
                help = true;
                break;
            }
            break;
        case 'm':        // with-mpirank-path
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(mpirank_path, optarg);
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
 * メイン関数
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @return                終了コード
 */
int main(int argc, char *argv[]) {
    UdmError_t ret;
    int myrank, num_procs;
    char dfiname[128] = {0x00};
    char output_path[128] = {"../output"};
    bool enable_hyper = true;
    bool enable_graph  = false;
    bool enable_partition = true;
    bool enable_repartition = false;
    int stepno = -1;
    char mpirank_path[512] = {0x00};

    printf("Start : partition\n");

    // 起動引数の取得
    if (!get_options(argc, argv,
                    dfiname,
                    output_path,
                    &enable_hyper,
                    &enable_graph,
                    &enable_partition,
                    &enable_repartition,
                    &stepno,
                    mpirank_path)) {
        print_help();
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (mpirank_path != NULL && strlen(mpirank_path) > 0) {
        sprintf(dfiname, "%s/%06d/index.dfi", mpirank_path, myrank);
    }
    else if (strlen(dfiname) == 0) {
        printf("Error : please input index.dfi\n");
        print_help();
        return 1;
    }

#if 0        // for debug
    MPI_Barrier(MPI_COMM_WORLD);
    udm_debug_setlevel(0x04);
    udm_debug_setoutput(0x03);        // 0x03 = コンソール、ファイル出力
#endif

    printf("Start :: loadModel!\n");
    // モデルの生成
    UdmHanler_t model = udm_create_model();
    if (udm_load_model(model, dfiname, 0) != UDM_OK) {
        printf("Error : can not load model[index.dfi=%s].\n", dfiname);
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: loadModel!\n");

    // 出力CGNSパス
    udm_config_setoutputpath(model, output_path);

    printf("Start :: Zoltan Partition!\n");
    if (enable_hyper && enable_partition) {
        // 分割パラメータ=ハイパーグラフ選択 : PARTITION
        udm_partition_sethypergraph(model, "PARTITION");
    }
    else if (enable_hyper && enable_repartition) {
        // 分割パラメータ=ハイパーグラフ選択 : REPARTITION
        udm_partition_sethypergraph(model, "REPARTITION");
    }
    else if (enable_graph && enable_partition) {
        // 分割パラメータ=グラフ選択 : PARTITION
        udm_partition_setgraph(model, "PARTITION");
    }
    else if (enable_graph && enable_repartition) {
        // 分割パラメータ=グラフ選択 : REPARTITION
        udm_partition_setgraph(model, "REPARTITION");
    }

    // Zoltanデバッグレベルを1に設定
    udm_partition_setdebuglevel(model, 1);

    // 分割実行
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        udm_delete_model(model);
        return -1;
    }
    int zone_id = 1;
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

    printf("Start :: writeModel!\n");
    // 分割ファイル出力 : step = 0
    ret = udm_write_model(model, 0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model.\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel!\n");

#ifdef _DEBUG
    if (num_procs > 1) {
        char rank_outpath[256] = {0x00};
        udm_config_getoutputdirectory(model, rank_outpath);
        // 内部境界出力
        char rank_filename[128] = {0x00};
        sprintf(rank_filename, "%s/rank_connectivity_id%08d.cgns", rank_outpath,  myrank);
        ret = udm_write_rankconnectivity(model, rank_filename);
        if (ret != UDM_OK) {
            printf("[for debug] Error : can not writeRankConnectivity[write_filename=%s]\n.", rank_filename);
        }

        // 仮想セル出力
        char virtual_filename[128] = {0x00};
        sprintf(virtual_filename, "%s/virtual_cells_id%08d.cgns", rank_outpath, myrank);
        ret = udm_write_virtualcells(model, virtual_filename);
        if (ret != UDM_OK) {
            printf("Error : can not writeVirtualCells[write_filename=%s].\n", virtual_filename);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // 内部境界の整合性チェック
    bool validate = udm_validate_cgns(model);
    if (validate) {
        printf("[rankno=%d] validateCgns : OK\n", myrank);
    }
    else {
        printf("[rankno=%d] validateCgns : NG\n", myrank);
        udm_delete_model(model);
        MPI_Abort(MPI_COMM_WORLD, -1);
        MPI_Finalize();
        return -1;
    }
#endif

#ifdef _UDM_PROFILER
    if (myrank == 0) {
        udm_stopwatch_print();
    }
#endif

    // 非構造格子モデル破棄
    udm_delete_model(model);
    MPI_Finalize();

    printf("End : partition\n");

    return 0;
}
