/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */


/**
 * DFIファイルに記述されている単位系の取得、設定を行う。
 * @file dfi_unit.c
 */

#include <udmlib.h>
#include <stdio.h>
#include <sys/stat.h>

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
    char dfiname[256] = {0x00};
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    int myrank, num_procs;
    const char usage[] = "usage: dfi_unit [DFI_FILE]";

    if (argc <= 1) {
        printf("Error : Please input DFI Filename.\n");
        printf("%s\n", usage);
        return 1;
    }

    // 読込ファイル名
    strcpy(dfiname, argv[1]);
    // ファイル名分解, 拡張子チェック
    udm_splitpath(dfiname, drive, folder, name, ext);
    if (strcmp(ext+1, "dfi") != 0) {
        printf("Error : Please input DFI Filename.\n");
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

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

    // 単位系
    // 単位系の追加
    // Length { Unit = "M",  Reference = 1.000000e-03}
    if (!udm_config_existsunit(model, "Length")) {
        udm_config_setunit(model, "Length", "m", 0.003);
        printf("add Unit : Length\n");

    }
    // Pressure { Unit = "Pa", Reference = 0.000000e+00, Difference = 5.100000e+02 }
    if (!udm_config_existsunit(model, "Pressure")) {
        udm_config_setunitwithdiff(model, "Pressure", "Pa", 0.000, 510.0);
        printf("add Unit : Pressure\n");
    }
    // Temperature { Unit = "C",  Reference = 1.000000e+01, Difference = 5.100000e+02 }
    if (!udm_config_existsunit(model, "Temperature")) {
        udm_config_setunitwithdiff(model, "Temperature", "C", 10.0, 0.1);
        printf("add Unit : Temperature\n");
    }
    // Velocity { Unit = "m/s", Reference = 3.400000e+00, }
    if (!udm_config_existsunit(model, "Velocity")) {
        udm_config_setunit(model, "Velocity", "m/s", 3.4);
        printf("add Unit : Velocity\n");
    }
    // Mass
    if (!udm_config_existsunit(model, "Mass")) {
        udm_config_setunit(model, "Mass", "kg", 0.0);
        printf("add Unit : Mass\n");
    }

    printf("Remove Unit : Mass\n");
    // 単位系:Massの削除
    udm_config_removeunit(model, "Mass");

    // 単位系の取得
    printf("print UnitList\n");
    char unit[128];
    float reference = 0.0, difference = 0.0;
    // Length
    if (udm_config_existsunit(model, "Length")) {
        unit[0] = 0x00;
        reference = difference = 0.0;
        udm_config_getunit(model, "Length", unit, &reference, &difference);
        printf("%s : unit=%s, Reference=%e\n", "Length", unit, reference);
    }
    // Pressure
    unit[0] = 0x00;
    reference = difference = 0.0;
    udm_config_getunit(model, "Pressure", unit, &reference, &difference);
    printf("%s : unit=%s, Reference=%e, Difference=%e\n", "Pressure", unit, reference, difference);

    // Temperature
    unit[0] = 0x00;
    reference = difference = 0.0;
    udm_config_getunit(model, "Temperature", unit, &reference, &difference);
    printf("%s : unit=%s, Reference=%e, Difference=%e\n", "Temperature", unit, reference, difference);

    // Velocity
    unit[0] = 0x00;
    reference = difference = 0.0;
    udm_config_getunit(model, "Velocity", unit, &reference, &difference);
    printf("%s : unit=%s, Reference=%e\n", "Velocity", unit, reference);

    // Mass
    if (udm_config_existsunit(model, "Mass")) {
        unit[0] = 0x00;
        reference = difference = 0.0;
        udm_config_getunit(model, "Mass", unit, &reference, &difference);
        printf("%s : unit=%s, Reference=%e\n", "Mass", unit, reference);
    }
    else {
        printf("%s : not exists[removed]. \n", "Mass");
    }

    // CGNSファイル出力 : step = 0
    printf("Start :: writeModel!\n");
    ret = udm_write_model(model, 0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model : none weight.\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel!\n");

    udm_delete_model(model);

    return 0;

}




