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
 * DFIファイルに記述されている単位系の取得、設定を行う。
 * @file dfi_unit.cpp
 */

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
    UdmModel *model = new UdmModel();
    if (model->loadModel(dfiname) != UDM_OK) {
        printf("Error : can not load model[index.dfi=%s].\n", dfiname);
        delete model;
        MPI_Finalize();
        return -1;
    }
    printf("End :: loadModel!\n");

    // 単位系設定
    UdmDfiConfig *config = model->getDfiConfig();
    UdmUnitListConfig *units_config = config->getUnitListConfig();

    // 単位系の追加
    // Length { Unit = "M",  Reference = 1.000000e-03}
    if (!units_config->existsUnitConfig("Length")) {
        units_config->setUnitConfig("Length", "m", 0.003);
        printf("add Unit : Length\n");

    }
    // Pressure { Unit = "Pa", Reference = 0.000000e+00, Difference = 5.100000e+02 }
    if (!units_config->existsUnitConfig("Pressure")) {
        units_config->setUnitConfig("Pressure", "Pa", 0.000, 510.0);
        printf("add Unit : Pressure\n");
    }
    // Temperature { Unit = "C",  Reference = 1.000000e+01, Difference = 5.100000e+02 }
    if (!units_config->existsUnitConfig("Temperature")) {
        units_config->setUnitConfig("Temperature", "C", 10.0, 0.1);
        printf("add Unit : Temperature\n");
    }
    // Velocity { Unit = "m/s", Reference = 3.400000e+00, }
    if (!units_config->existsUnitConfig("Velocity")) {
        units_config->setUnitConfig("Velocity", "m/s", 3.4);
        printf("add Unit : Velocity\n");
    }
    // Mass
    if (!units_config->existsUnitConfig("Mass")) {
        units_config->setUnitConfig("Mass", "kg", 0.0);
        printf("add Unit : Mass\n");
    }

    printf("Remove Unit : Mass\n");
    // 単位系:Massの削除
    units_config->removeUnitConfig("Mass");

    // 単位系の取得
    printf("print UnitList\n");
    std::string unit;
    float reference = 0.0, difference = 0.0;
    // Length
    if (units_config->existsUnitConfig("Length")) {
        unit = "";
        reference = difference = 0.0;
        units_config->getUnit("Length", unit);
        units_config->getReference("Length", reference);
        printf("%s : unit=%s, Reference=%e\n", "Length", unit.c_str(), reference);
    }
    // Pressure
    unit = "";
    reference = difference = 0.0;
    units_config->getUnit("Pressure", unit);
    units_config->getReference("Pressure", reference);
    units_config->getDifference("Pressure", difference);
    printf("%s : unit=%s, Reference=%e, Difference=%e\n", "Pressure", unit.c_str(), reference, difference);

    // Temperature
    unit = "";
    reference = difference = 0.0;
    units_config->getUnit("Temperature", unit);
    units_config->getReference("Temperature", reference);
    units_config->getDifference("Temperature", difference);
    printf("%s : unit=%s, Reference=%e, Difference=%e\n", "Temperature", unit.c_str(), reference, difference);

    // Velocity
    unit = "";
    reference = difference = 0.0;
    units_config->getUnit("Velocity", unit);
    units_config->getReference("Velocity", reference);
    printf("%s : unit=%s, Reference=%e\n", "Velocity", unit.c_str(), reference);

    // Mass
    if (units_config->existsUnitConfig("Mass")) {
        unit = "";
        reference = difference = 0.0;
        units_config->getUnit("Mass", unit);
        units_config->getReference("Mass", reference);
        printf("%s : unit=%s, Reference=%e\n", "Mass", unit.c_str(), reference);
    }
    else {
        printf("%s : not exists[removed]. \n", "Mass");
    }

    // CGNSファイル出力 : step = 0
    printf("Start :: writeModel!\n");
    ret = model->writeModel(0, 0.0);
    if (ret != UDM_OK) {
        cout << "Error : can not write model" << endl;
        delete model;
        MPI_Finalize();
        return -1;
    }
    printf("End :: writeModel!\n");

    delete model;

    return 0;

}




