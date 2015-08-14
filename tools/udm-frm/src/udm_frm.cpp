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
 * @file   udm_frm.cpp
 * @brief  main関数
 */

#include <stdio.h>
#include <iostream>
#include <getopt.h>
#include <sys/stat.h>
#include <udmlib.h>
#include <model/UdmModel.h>
#include "UdmStaging.h"
#include "udmfrm_version.h"

using namespace std;
using namespace udm;

void print_help();
bool set_arg(UdmStaging &staging, int argc, char **argv);

/**
 * メイン関数
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @return                終了コード
 */
int main( int argc, char **argv )
{
    UdmStaging staging;

    //引数の取り出し＆セット
    if (!set_arg(staging,argc,argv)) {
        print_help();
        return 1;
    }
    // バージョン表示
    if (staging.isPrintVersion()) {
        printf("[udm-frm] Version = %s, Revision = %s.\n", UDMFRM_VERSION, UDMFRM_REVISION);
        return 0;
    }
    // index.dfi
    if ( staging.getIndexDfi().empty()) {
        printf("ERROR : undefined input file[--input].\n");
        print_help();
        return 1;
    }
    // np
    if (staging.getNumProcess() <= 0) {
        printf("ERROR : undefined number of process[--np]. \n");
        print_help();
        return 1;
    }

#ifndef WITHOUT_MPI
    //MPI Initialize
    if ( MPI_Init(&argc,&argv) != MPI_SUCCESS ) {
        printf("ERROR : MPI_Init error. \n");
        return 1;
    }
#endif

    printf("[udm-frm] start.\n");
    staging.print_info();

    printf("[udm-frm] readDfi.\n");
    // index.dfiファイルの読込み
    if ( !staging.readDfi() ) {
        printf("ERROR : can not read index.dfi[%s]. \n", staging.getIndexDfi().c_str());
#ifndef WITHOUT_MPI
        MPI_Finalize();
#endif
        return 1;
    }

    printf("[udm-frm] mappingFiles.\n");
    // ファイルの振分を行う.
    if ( !staging.mappingFiles() ) {
        printf("ERROR : can not file mapping. \n");
#ifndef WITHOUT_MPI
        MPI_Finalize();
#endif
        return 1;
    }

#ifndef WITHOUT_MPI
    MPI_Finalize();
#endif
    printf("[udm-frm] success.\n");
    return 0;
}

/**
 * 引数からステージングパラメータを取得する。
 * @param [out] staging        ステージングパラメータ
 * @param [in] argc            起動引数の数
 * @param [in] argv            起動引数の文字列
 */
bool set_arg(UdmStaging &staging, int argc, char **argv)
{

    struct option long_options[] = {
        {"input", required_argument,        NULL, 'i'},
        {"output", required_argument,        NULL, 'o'},
        {"np",  required_argument,  NULL, 'n'},
        {"with-udmlib", optional_argument,  NULL, 'u'},
        {"step",  required_argument,  NULL, 's'},
        {"view",  no_argument,  NULL, 'v'},
        {"version",  no_argument,  NULL, 'V'},
        {"help",  no_argument,  NULL, 'h'},
        {0, 0, 0, 0}
    };

    bool help = false;
    int opt, option_index;
    int num_procs = 0;
    int stepno = 0;
    bool with_udmlib = false;
    while ((opt = getopt_long(argc, argv, "i:o:n:u:s:vVh", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'i':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            staging.setIndexDfi(optarg);
            break;
        case 'o':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            staging.setOutputDirectory(optarg);
            break;
        case 'n':
            if (optarg == NULL || strlen(optarg) <= 0) {
                help = true;
                break;
            }
            num_procs = atoi(optarg);
            if (num_procs > 0) {
                staging.setNumProcess(num_procs);
            }
            else {
                help = true;
                break;
            }
            break;
        case 'u':
            if (optarg != NULL && strlen(optarg) > 0) {
                staging.setUdmlibFile(optarg);
            }
            with_udmlib = true;
            break;
        case 's':
            stepno = atoi(optarg);
            if (stepno < 0) {
                printf("Error : stepno less than zero.");
                help = true;
                break;
            }
            staging.setStepno(stepno);
            break;
        case 'v':
            staging.setDebugTrace(true);
            break;
        case 'V':
            staging.setPrintVersion(true);
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

    if (help) {
        return !help;
    }

    // index.dfiのパスからudmlib.tpを取得する
    if (with_udmlib && staging.getUdmlibFile().empty()) {
        // index.dfiのファイル名
        std::string index_dfi = staging.getIndexDfi();
        char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
        // ファイル名分解
        udm_splitpath(index_dfi.c_str(), drive, folder, name, ext);

        // udmlib.tpパス
        char udmlib_path[1024] = {0x00};
        udm_connect_path(folder, UDM_DFI_FILENAME_UDMLIB, udmlib_path);
        staging.setUdmlibFile(udmlib_path);
    }

    return !help;
}

/**
 * ヘルプの表示
 */
void print_help()
{
    const char usage[] = "usage: udm-frm --input INDEX_DFI --np N  OPTIONS.";
    printf("%s\n", usage);
    printf("OPTIONS:\n");
    printf("    -i, --input [=INDEX_DFI]              入力index.dfiファイル\n");
    printf("    -n, --np [=N]                         振分プロセス数 \n");
    printf("    -o, --output [=OUTPUT_PATH]           出力ディレクトリ \n");
    printf("                                          デフォルト = ./ \n");
    printf("    -u, --with-udmlib [=UDMLIBTP_FILE]    udmlib.tpファイル \n");
    printf("    -s, --step [=STEP_NO]                 ファイルコピーステップ番号 \n");
    printf("    -v --view                             ファイルコピー表示\n");
    printf("    --version                             バージョン情報表示\n");
    printf("    -h --help                             ヘルプ出力\n");
    printf("(例)\n");
    printf("    udm-frm --input=model_hexa/index.dfi --np=4 --output=model_p4 \n");
    printf("\n");

    return;
}

