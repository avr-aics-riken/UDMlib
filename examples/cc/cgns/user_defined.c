/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file user_defined.c
 * ユーザ定義CGNS要素の取得、設定を行う。
 */


#include <udmlib.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef __cplusplus
using namespace udm;
#endif

/**
 * CGNSファイルから読込を行う.
 * @param filename        CGNSファイル名
 */
void readCgns(const char *filename)
{
    UdmDataType_t datatype;
    int dimension = 0, n;
    UdmSize_t dim_sizes[3] = {0};
    int len;
    UdmSize_t i, j;

    printf("/**** read user defined data ****/ \n");
    printf("cgns file = %s\n", filename);

    // モデルの生成
    UdmHanler_t model = udm_create_model();

    // CGNSファイル読込
    udm_read_cgns(model, filename, 0);
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        udm_delete_model(model);
        return;
    }
    int zone_id = 1;

    // ユーザ定義データ
    udm_user_getinfo(model, zone_id, "UserMatrix", &datatype, &dimension, dim_sizes);

    len = 1;
    for (n=0; n<dimension; n++) {
        len *= dim_sizes[n];
    }
    float *mat_values = (float*)malloc(sizeof(float)*len);
    udm_user_getdata(model, zone_id, "UserMatrix", Udm_RealSingle, mat_values);
    // 表示
    printf("UserMatrix\n");
    printf("    size = %ld x %ld \n", (long long)dim_sizes[0], (long long)dim_sizes[1]);
    n = 0;
    printf("    value = ");
    for (j=0; j<dim_sizes[1]; j++) {
        for (i=0; i<dim_sizes[0]; i++) {
            printf("%f ", mat_values[n++]);
        }
        printf("\n");
        printf("            ");
    }
    printf("\n");

    udm_user_getinfo(model, zone_id, "UserArray", &datatype, &dimension, dim_sizes);
    len = 1;
    for (n=0; n<dimension; n++) {
        len *= dim_sizes[n];
    }
    int *array_values = (int*)malloc(sizeof(int)*len);
    udm_user_getdata(model, zone_id, "UserArray", Udm_Integer, array_values);
    // 表示
    printf("UserArray\n");
    printf("    size = %ld \n", (long long)dim_sizes[0]);
    n = 0;
    printf("    value = ");
    for (i=0; i<dim_sizes[0]; i++) {
        printf("%d ", array_values[n++]);
    }
    printf("\n");

    free(mat_values);
    free(array_values);

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
    char dfiname[256] = {0x00};
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    int myrank, num_procs;
    const char usage[] = "usage: user_defined [DFI_FILE]";
    UdmSize_t i, j;

    printf("Start : user_defined\n");

    if (argc <= 1) {
        printf("Error : Please input DFI Filename.\n");
        printf("%s\n", usage);
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // 読込ファイル名
    strcpy(dfiname, argv[1]);
    // ファイル名分解, 拡張子チェック
    udm_splitpath(dfiname, drive, folder, name, ext);
    if (strcmp(ext+1, "dfi") != 0) {
        printf("Error : Please input DFI Filename.\n");
        return 1;
    }

    printf("Start :: loadModel!\n");
    // モデルの生成
    UdmHanler_t model = udm_create_model();

    if (udm_load_model(model, dfiname, -1) != UDM_OK) {
        printf("Error : can not load model[index.dfi=%s].\n", dfiname);
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: loadModel!\n");

    // ゾーンの取得
    if (udm_getnum_zones(model) <= 0) {
        printf("Error : can not get zone.\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    int zone_id = 1;        // 最初のゾーンとする

    // ユーザ定義データの設定
    printf("/**** write user defined data ****/ \n");
    UdmSize_t user_sizes[2] = {3, 10};
    float user_real[10][3];
    int value = 0;
    printf("UserMatrix\n");
    printf("    size = %ld x %ld \n", (long long)user_sizes[0], (long long)user_sizes[1]);
    printf("    value = ");
    for (j=0; j<user_sizes[1]; j++) {
        for (i=0; i<user_sizes[0]; i++) {
            user_real[j][i] = 0.5*(value++);
            printf("%f ", user_real[j][i]);
        }
        printf("\n            ");
    }
    printf("\n");
    udm_user_setdata(model, zone_id, "UserMatrix", Udm_RealSingle, 2, user_sizes, user_real);

    UdmSize_t size = 5;
    int user_int[5] = {1, 3, 5, 7, 9};
    printf("UserArray\n");
    printf("    size = %ld \n", (long long)size);
    printf("    value = ");
    for (i=0; i<size; i++) {
        printf("%d ", user_int[i]);
    }
    printf("\n");
    udm_user_setdata(model, zone_id, "UserArray", Udm_Integer, 1, &size, user_int);

    // CGNSファイル出力 : step = 0
    printf("Start :: writeModel!\n");
    ret = udm_write_model(model, 0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel!\n");

    // 出力CGNSファイルの読込
    char outout_cgns[512] = {0x00};
    udm_config_getcgnslinkfile(model, outout_cgns, 0);
    readCgns(outout_cgns);

    printf("End : user_defined\n");

    return 0;
}



