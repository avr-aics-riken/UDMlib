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
 * @file create_timeslice.cpp
 * TimeSlice出力のFileCompositionTypeを設定する.
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include "udmlib.h"
#include "model/UdmModel.h"


using namespace std;
using namespace udm;

/**
 * ヘルプ表示
 */
void print_help()
{
    const char usage[] = "usage: create_timeslice --input=[INDEX_DFI] --output=[OUTPUT_PATH] OPTIONS.";
    printf("%s\n", usage);
    printf("OPTIONS:\n");
    printf("    --input=[INDEX_DFI]                    入力INDEX_DFIファイル\n");
    printf("    --output=[OUTPUT_PATH]                 出力パス\n");
    printf("    /**** CGNS:GridCoordinates/FlowSolution出力 ****/ \n");
    printf("    --includegrid         CGNS:GridCoordinatesとCGNS:FlowSolutionを１つのファイルに出力します。(デフォルト) \n");
    printf("    --excludegrid         CGNS:GridCoordinatesとCGNS:FlowSolutionを別ファイルに出力します。\n");
    printf("    /**** CGNS:FlowSolution時系列出力 ****/ \n");
    printf("    --appendstep         CGNS:FlowSolutionを時系列毎に１つのファイルに出力します。 \n");
    printf("    --eachstep           CGNS:FlowSolutionを時系列毎に別ファイルにします。(デフォルト) \n");
    printf("    /**** CGNS:GridCoordinates時系列出力 ****/ \n");
    printf("    --gridconstant        CGNS:GridCoordinatesは初期値のみ出力を行います。(デフォルト) \n");
    printf("    --gridtimeslice        CGNS:GridCoordinatesは時系列毎に出力を行います。\n");
    printf("    /**** output directory options ****/ \n");
    printf("    --with_directorypath=[DIR]        フィールド出力ディレクトリ \n");
    printf("    --with_timeslice_directory        時系列ディレクトリ作成 \n");
    printf("    /***************************************/ \n");
    printf("    -h --help                                     ヘルプ出力\n");
    printf("(例)\n");
    printf("    create_timeslice --input=input/index.dfi --output=includegrid_eachstep_gridtimeslice --includegrid  --eachstep --gridtimeslice \n");
    printf("\n");
    return;
}

/**
 * 起動オプションを取得する.
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @param filename        入力DFIファイル名
 * @param output_path     出力パス
 * @param includegrid     CGNS:GridCoordinates/FlowSolution出力設定
 * @param eachstep         CGNS:FlowSolution時系列出力
 * @param gridconstant      CGNS:GridCoordinates時系列出力
 * @param directorypath      フィールド出力ディレクトリ
 * @param timeslice_directory      時系列ディレクトリ作成
 * @return                取得成功
 */
bool get_options(int argc, char *argv[],
                char *filename,
                char *output_path,
                bool &includegrid,
                bool &eachstep,
                bool &gridconstant,
                char *directorypath,
                bool &timeslice_directory)
{
    struct option long_options[] = {
        {"input", required_argument,        NULL, 'n'},
        {"output", required_argument,        NULL, 'o'},
        {"includegrid", no_argument,        NULL, 'i'},
        {"excludegrid", no_argument,        NULL, 'x'},
        {"appendstep", no_argument,        NULL, 'a'},
        {"eachstep", no_argument,        NULL, 'e'},
        {"gridconstant",  no_argument,  NULL, 'c'},
        {"gridtimeslice",  no_argument,  NULL, 't'},
        {"with_directorypath",  required_argument,  NULL, 'f'},
        {"with_timeslice_directory",  no_argument,  NULL, 'd'},
        {"help",  no_argument,  NULL, 'h'},
        {0, 0, 0, 0}
    };

    bool help = false;
    int opt, option_index;
    while ((opt = getopt_long(argc, argv, ":n:i:x:a:e:c:t:f:d", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'n':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(filename, optarg);
            break;
        case 'o':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(output_path, optarg);
            break;
        case 'i':        // includegrid
            includegrid = true;
            break;
        case 'x':        // excludegrid
            includegrid = false;
            break;
        case 'a':        // appendstep
            eachstep = false;
            break;
        case 'e':        // eachstep
            eachstep = true;
            break;
        case 'c':        // gridconstant
            gridconstant = true;
            break;
        case 't':        // gridtimeslice
            gridconstant = false;
            break;
        case 'f':        // with_directorypath
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            strcpy(directorypath, optarg);
            break;
        case 'd':        // with_timeslice_directory
            timeslice_directory = true;
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
    char filename[256] = {0x00};
    char outputname[256] = {0x00};
    char outputpath[256] = {0x00};
    int myrank, num_procs;
    bool includegrid = true;
    bool eachstep = false;
    bool gridconstant = true;
    char directorypath[256] = {0x00};
    bool timeslice_directory = false;
    char path[1024] = {0x00};
    printf("Start : create_timeslice\n");

    // 起動引数の取得
    if (!get_options(argc, argv, filename, outputname, includegrid, eachstep, gridconstant, directorypath, timeslice_directory)) {
        print_help();
        return 1;
    }
    if (strlen(filename) == 0) {
        printf("Error : please input index.dfi\n");
        print_help();
        return 1;
    }
    if (strlen(outputname) == 0) {
        printf("Error : please output path\n");
        print_help();
        return 1;
    }

    printf("DFI Filename : %s\n", filename);
    printf("CGNS:GridCoordinates/FlowSolution出力\n");
    if (includegrid) printf("    CGNS:GridCoordinatesとCGNS:FlowSolutionを１つのファイルに出力します。\n");
    else printf("    CGNS:GridCoordinatesとCGNS:FlowSolutionを別ファイルに出力します。\n");

    printf("CGNS:FlowSolution時系列出力\n");
    if (!eachstep) printf("    CGNS:FlowSolutionを時系列毎に１つのファイルに出力します。\n");
    else printf("    CGNS:FlowSolutionを時系列毎に別ファイルにします。\n");
    printf("CGNS:GridCoordinates時系列出力\n");
    if (gridconstant) printf("    CGNS:GridCoordinatesは初期値のみ出力を行います。\n");
    else printf("    CGNS:GridCoordinatesは時系列毎に出力を行います。\n");
    if (strlen(directorypath) > 0 || timeslice_directory) {
        printf("Output Directory Options \n");
        if (strlen(directorypath) > 0) printf("    フィールドディレクトリ[%s]に出力を行います。\n", directorypath);
        if (timeslice_directory) printf("    時系列毎のディレクトリに出力を行います。\n");
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    printf("loadModel!\n");
    // モデルの生成
    UdmModel *model = new UdmModel();
    // モデル読込
    if (model->loadModel(filename) != UDM_OK) {
        printf("Error : can not load model[index.dfi=%s].\n", filename);
        delete model;
        MPI_Finalize();
        return -1;
    }

    // カレントディレクトリ
    getcwd(path, 1024);
    sprintf(outputpath, "%s/%s/", path, outputname);

    // CGNS構成
    UdmDfiConfig* config = model->getDfiConfig();
    if (includegrid) {
        config->getFileinfoConfig()->setFileCompositionType(Udm_IncludeGrid);
    }
    else {
        config->getFileinfoConfig()->setFileCompositionType(Udm_ExcludeGrid);
    }
    if (eachstep) {
        config->getFileinfoConfig()->setFileCompositionType(Udm_EachStep);
    }
    else {
        config->getFileinfoConfig()->setFileCompositionType(Udm_AppendStep);
    }
    if (gridconstant) {
        config->getFileinfoConfig()->setFileCompositionType(Udm_GridConstant);
    }
    else {
        config->getFileinfoConfig()->setFileCompositionType(Udm_GridTimeSlice);
    }
    config->getFileinfoConfig()->setDirectoryPath(directorypath);
    config->getFileinfoConfig()->setTimeSliceDirectory(timeslice_directory);


    // 物理量の設定
    UdmFlowSolutionListConfig* solutions = config->getFlowSolutionListConfig();
    solutions->setSolutionFieldInfo("Pressure", Udm_Vertex, Udm_RealSingle);

    // モデル出力
    printf("writeModel!\n");
    model->setOutputPath(outputpath);
    ret = model->writeModel(0, 0.0);
    if (ret != UDM_OK) {
        cout << "Error : can not write model" << endl;
        delete model;
        MPI_Finalize();
        return -1;
    }

    // 非構造格子モデル破棄
    delete model;

    cout << "End : create_timeslice" <<endl;

    return 0;
}



