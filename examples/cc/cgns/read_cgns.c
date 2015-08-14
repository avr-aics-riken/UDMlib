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
 * @file read_cgns.c
 * CGNSファイルを読み込む。
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "udmlib.h"

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
    int n;
    char filename[256] = {0x00};
    char output_path[256] = {0x00};
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    int myrank, num_procs;
    const char usage[] = "usage: read_cgns [CGNS_FILE]";

    if (argc <= 1) {
        printf("Error : Please input CGNS Filename.\n");
        printf("%s\n", usage);
        return 1;
    }

    // 読込ファイル名
    strcpy(filename, argv[1]);
    // ファイル名分解, 拡張子チェック
    udm_splitpath(filename, drive, folder, name, ext);
    if (strcmp(ext+1, "cgns") != 0) {
        printf("Error : not support %s, only cgns.\n", ext+1);
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    printf("Start :: readCgns!\n");
    // モデルの生成
    UdmHanler_t model = udm_create_model();

    // CGNSファイル読込
    ret = udm_read_cgns(model, filename, -1);
    if (ret != UDM_OK) {
        // CGNSファイル読込失敗
        printf("Error : can not open CGNS file[%s]\n", filename);
        udm_delete_model(model);
        MPI_Finalize();
        return ret;
    }
    printf("End :: readCgns!\n");

    // 出力設定
    udm_config_setfileprefix(model, name);
    udm_config_setoutputpath(model, output_path);

    // CGNSファイル出力 : step = 0
    printf("Start :: writeModel!\n");
    ret = udm_write_model(model, 0, 0.0);

    printf("End :: writeModel!\n");

    udm_delete_model(model);
    MPI_Finalize();
    return 0;

}
