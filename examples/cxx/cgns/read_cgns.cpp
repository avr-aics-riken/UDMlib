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
 * @file read_cgns.cpp
 * CGNSファイルを読み込む。
 */

#include <udmlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <model/UdmModel.h>
#include <model/UdmZone.h>
#include <model/UdmFlowSolutions.h>
#include <model/UdmSections.h>
#include <model/UdmGridCoordinates.h>

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
    int n;
    char filename[256] = {0x00};
    char output_path[256] = {0x00};
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    int myrank, num_procs;
    string buf;
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

    // モデルの生成
    UdmModel *model = new UdmModel();
    UdmDfiConfig* config = model->getDfiConfig();

    // CGNSファイル読込
    ret = model->readCgns(filename);
    if (ret != UDM_OK) {
        // CGNSファイル読込失敗
        cout << "Error : can not open CGNS file" << "[" << filename << "]";
        delete model;
        return ret;
    }

    // CGNS情報出力
    model->toString(buf);
    printf(buf.c_str());

    // 出力設定
    config->setFileInfoPrefix(name);
    sprintf(output_path, "output");
    config->setOutputPath(output_path);

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
