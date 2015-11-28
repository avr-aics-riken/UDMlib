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
 * @file create_cgns.cpp
 * 6面体CGNSモデルを作成する.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include "udmlib.h"
#include "model/UdmModel.h"


using namespace std;
using namespace udm;

#define VOXEL_I_SIZE        32
#define VOXEL_J_SIZE        32
#define VOXEL_K_SIZE        32

/**
 * ヘルプ表示
 */
void print_help()
{
    const char usage[] = "usage: create_cgns --name [CGNS_FILENAME] --size [X_SIZE] [Y_SIZE] [Z_SIZE] OPTIONS.";
    printf("%s\n", usage);
    printf("OPTIONS:\n");
    printf("    -n, --name=[CGNS_FILENAME]                    出力CGNSファイル名\n");
    printf("                                                  デフォルト = cgns_model.cgns \n");
    printf("    -s, --size [X_SIZE],[Y_SIZE],[Z_SIZE]         X,Y,Zサイズ\n");
    printf("                                                  デフォルト = 16,16,16 \n");
    printf("    -c, --coords [X_COORD],[Y_COORD],[Z_COORD]    X,Y,Z座標幅\n");
    printf("                                                  デフォルト = 1.0,1.0,1.0\n");
    printf("    -h --help                                     ヘルプ出力\n");
    printf("(例)\n");
    printf("    create_cgns --name=output/cgns_hexa.cgns --size=32,32,32 --coords=1.0,1.5,2.0 \n");
    printf("\n");
    return;
}

/**
 * 起動オプションを取得する.
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @param filename        出力CGNSファイル名
 * @param size            X,Y,Zサイズ
 * @param coords        X,Y,Z座標幅
 * @return                取得成功
 */
bool get_options(int argc, char *argv[], char *filename, int size[3], float coords[3])
{
    struct option long_options[] = {
        {"name", required_argument,        NULL, 'n'},
        {"size", required_argument,        NULL, 's'},
        {"coords",  required_argument,  NULL, 'c'},
        {"help",  no_argument,  NULL, 'h'},
        {0, 0, 0, 0}
    };

    bool help = false;
    int opt, option_index;
    char *tok;
    while ((opt = getopt_long(argc, argv, ":n:s:c:", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'n':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(filename, optarg);
            break;
        case 's':
            if (strlen(optarg) <= 0) {
                help = true;
            }
            tok = strtok( optarg, "," );
            if (tok == NULL || strlen(tok) <= 0) {help = true; break;};
            size[0] = atoi(tok);
            tok = strtok( NULL, "," );
            if (tok == NULL || strlen(tok) <= 0) {help = true; break;};
            size[1] = atoi(tok);
            tok = strtok( NULL, "," );
            if (tok == NULL || strlen(tok) <= 0) {help = true; break;};
            size[2] = atoi(tok);
            break;
        case 'c':
            if (strlen(optarg) <= 0) {
                help = true;
            }
            tok = strtok( optarg, "," );
            if (tok == NULL || strlen(tok) <= 0) {help = true; break;};
            coords[0] = atof(tok);
            tok = strtok( NULL, "," );
            if (tok == NULL || strlen(tok) <= 0) {help = true; break;};
            coords[1] = atof(tok);
            tok = strtok( NULL, "," );
            if (tok == NULL || strlen(tok) <= 0) {help = true; break;};
            coords[2] = atof(tok);
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
    char filename[256] = "cgns_model.cgns";
    int grid_size[3] = {VOXEL_I_SIZE, VOXEL_J_SIZE, VOXEL_K_SIZE};
    float coords[3] = {1.0, 1.0, 1.0};
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    int myrank, num_procs;
    int n;

    printf("Start : create_cgns\n");

    // 起動引数の取得
    if (!get_options(argc, argv, filename, grid_size, coords)) {
        print_help();
        return 1;
    }
    printf("CGNS Filename : %s\n", filename);
    printf("GridCoordinates Size : %d,%d,%d\n", grid_size[0], grid_size[1], grid_size[2]);
    printf("Cell Coordinate : %f,%f,%f\n", coords[0], coords[1], coords[2]);

    // ファイル名分解, 拡張子チェック
    udm_splitpath(filename, drive, folder, name, ext);
    if (strcmp(ext+1, "cgns") != 0) {
        printf("Error : not support %s, only cgns.\n", ext+1);
        return 1;
    }
    if (strlen(folder) == 0) {
        strcpy(folder, "output");
    }

    // グリッドサイズチェック
    for (n=0; n<3; n++) {
        if (grid_size[n] <= 1) {
            printf("Error : GridCoordinates size must is more than 2 [%d].\n", grid_size[n]);
            return 1;
        }
        if (coords[n] <= 0.0) {
            printf("Error : Cell Coordinate is zero [%f].\n", coords[n]);
            return 1;
        }
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // モデルの生成
    UdmModel *model = new UdmModel();
    UdmDfiConfig* config = model->getDfiConfig();

    config->setFileInfoPrefix(name);
    config->setOutputPath(folder);

    // モデルの作成

    int i, j, k;
    UdmSize_t node_id;
    UdmSize_t elem_nodes[8];

    // ゾーンの作成
    UdmZone *zone = model->createZone();

    // 座標の設定
    UdmGridCoordinates *grid = zone->getGridCoordinates();
    for (k=0; k<grid_size[2]; k++) {
        for (j=0; j<grid_size[1]; j++) {
            for (i=0; i<grid_size[0]; i++) {
                float x = i*coords[0];
                float y = j*coords[1];
                float z = k*coords[2];
                node_id = grid->insertGridCoordinates(x, y, z);
            }
        }
    }

    // HEX_8要素の設定
    UdmSections *sections = zone->getSections();
    UdmElements *elements = sections->createSection(Udm_HEXA_8);
    for (k=0; k<grid_size[2]-1; k++) {
        for (j=0; j<grid_size[1]-1; j++) {
            for (i=0; i<grid_size[0]-1; i++) {
                node_id =i+j*grid_size[0]+k*grid_size[0]*grid_size[1] + 1;
                elem_nodes[0]=node_id;
                elem_nodes[1]=node_id+1;
                elem_nodes[2]=node_id+1+grid_size[0];
                elem_nodes[3]=node_id+grid_size[0];
                elem_nodes[4]=node_id+grid_size[0]*grid_size[1];
                elem_nodes[5]=node_id+grid_size[0]*grid_size[1]+1;
                elem_nodes[6]=node_id+grid_size[0]*grid_size[1]+1+grid_size[0];
                elem_nodes[7]=node_id+grid_size[0]*grid_size[1]+grid_size[0];
                // 要素の追加
                elements->insertCellConnectivity(Udm_HEXA_8, elem_nodes);
            }
        }
    }

    // 分割ファイル出力 : step = 0
    ret = model->writeModel(0, 0.0);
    if (ret != UDM_OK) {
        cout << "Error : can not write model" << endl;
        delete model;
        MPI_Finalize();
        return -1;
    }

    // 非構造格子モデル破棄
    delete model;
    MPI_Finalize();

    cout << "End : create_cgns" <<endl;

    return 0;
}



