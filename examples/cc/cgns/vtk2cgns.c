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
 * @file vtk2cgns.c
 * VTKフォーマットのグリッドデータを読み込み、分割、CGNS出力を行う。
 */


#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <udmlib.h>

#ifdef __cplusplus
using namespace udm;
#endif

/**
 * VTK:CELL情報
 */
struct vtk_cell_t {
    int num_cell_points;
    int *cell_points;
    int cell_type;
};

/**
 * VTK情報
 */
struct vtk_t {
    int num_points;
    float (*coords)[3];
    int num_cells;
    struct vtk_cell_t *vtk_cells;
};

/**
 * 文字列を小文字に変換する.
 * @param str        変換文字列
 * @return            変換文字列先頭ポインタ
 */
char *string_tolower(char *str)
{
    char *p;
    for (p = str; *p; p++) *p = tolower(*p);
    return (str);
}

/**
 * 文字列を大文字に変換する.
 * @param str        変換文字列
 * @return            変換文字列先頭ポインタ
 */
char *string_toupper(char *str)
{
    char *p;
    for (p = str; *p; p++) *p = toupper(*p);
    return (str);
}

/**
 * 文字列の前後から空白、タブ、改行を削除する.
 * @param str        削除対象文字列
 * @return            空白除去文字列ポインタ
 */
char *trim(char *str)
{
    char *end;
    while(isspace(*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;
    *(end+1) = 0x00;

    return str;
}

/**
 * 文字列を分割する
 * @param line        文字列
 * @param list        分割文字列リスト（MAX16)
 * @return        分割数
 */
int split_string(char* line, char *list[16])
{
    char delim[] = " ,\t";
    char    *tok;
    int     count = 0;
    trim(line);
    tok = strtok( line, delim );
    while( tok != NULL) {
        list[count++] = tok;
        tok = strtok( NULL, delim );
        if (count >= 16) break;
    }
    return count;
}


/**
 * 文字列を実数に分割する
 * @param line        文字列
 * @param real_list        分割実数リスト
 * @return        分割数
 */
int split_real(char* line, float *real_list, int size)
{
    char *list[16];
    int n;
    trim(line);
    int count = split_string(line, list);
    for (n=0; n<count; n++) {
        real_list[n] = atof(list[n]);
        if (n >= size) break;
    }
    return n;
}


/**
 * 文字列を整数に分割する
 * @param line        文字列
 * @param int_list        分割整数リスト
 * @return        分割数
 */
int split_integer(char* line, int *int_list, int size)
{
    char *list[16];
    int n;
    trim(line);
    int count = split_string(line, list);
    for (n=0; n<count; n++) {
        int_list[n] = atoi(list[n]);
        if (n >= size) break;
    }
    return n;
}

/**
 * VTK:POINTSを読み込む
 * @param coords        VTK:POINTSリスト
 * @param num_points    VTK:POINTS数
 * @param fp        ファイルポインタ
 * @return        読込VTK:POINTS数
 */
int read_points(float (*coords)[3], int num_points, FILE *fp)
{
    char line[256];
    int count = 0;
    int size = 0;
    float real_list[3];
    int n;

    // POINTS
    while( fgets( line, 256, fp ) != NULL ) {
        size = split_real(line, real_list, 3);
        if (size != 3) return -1;
        float *p = (float*)(coords + count);
        for (n=0; n<3; n++) {
            p[n] = real_list[n];
        }
        count++;
        if (count >= num_points) {
            break;
        }
    }

    return count;
}


/**
 * VTK:CELLSを読み込む
 * @param vtk_cells        VTK:CELLSリスト
 * @param num_cells    VTK:CELLS数
 * @param fp        ファイルポインタ
 * @return        読込VTK:CELLS数
 */
int read_cells(struct vtk_cell_t *vtk_cells, int num_cells, FILE *fp)
{
    char line[256];
    int count = 0;
    int size = 0;
    int int_list[16];
    int n;

    // CELLS
    while( fgets( line, 256, fp ) != NULL ) {
        size = split_integer(line, int_list, 16);
        if (size <= 1) return -1;
        if (size != int_list[0] + 1) return -1;
        vtk_cells[count].num_cell_points = int_list[0];
        vtk_cells[count].cell_points = (int*)malloc(sizeof(int)*int_list[0]);
        for (n=0; n<int_list[0]; n++) vtk_cells[count].cell_points[n] = int_list[n+1];
        count++;
        if (count >= num_cells) {
            break;
        }
    }

    return count;
}


/**
 * VTK:CELL_TYPESを読み込む
 * @param vtk_cells        VTK:CELL_TYPESリスト
 * @param num_cells    VTK:CELL_TYPES数
 * @param fp        ファイルポインタ
 * @return        読込VTK:CELL_TYPES数
 */
int read_cell_types(struct vtk_cell_t *vtk_cells, int num_cells, FILE *fp)
{
    char line[256];
    int count = 0;
    int size = 0;
    int int_list[16];
    int n;

    // CELL_TYPES
    while( fgets( line, 256, fp ) != NULL ) {
        size = split_integer(line, int_list, 16);
        if (size <= 0) return -1;
        vtk_cells[count].cell_type = int_list[0];
        count++;
        if (count >= num_cells) {
            break;
        }
    }

    return count;
}

/**
 * VTKファイルから読み込みを行う.
 * @param [in]  filename        VTKファイル名
 * @param [out] vtk_info        VTK情報
 * @return        0=正常
 */
int read_vtk(const char *filename, struct vtk_t *vtk_info)
{
    FILE *fp;
    char line[256];
    int n;
    char *split_list[16];
    int count = 0;

    vtk_info->num_points = 0;
    vtk_info->num_cells = 0;
    vtk_info->coords = NULL;
    vtk_info->vtk_cells = NULL;

    fp = fopen( filename, "r" );
    if( fp == NULL ){
        printf( "Error : can not open [filename=%s].\n", filename);
        return -1;
    }

    // 先頭行読み込み
    if (fgets( line, 256, fp ) == NULL) {
        printf( "Error : can not read \"vtk DataFile Version\" [filename=%s].\n", filename);
        return -1;
    }
    trim(line);
    if (udm_strcasestr(line, "vtk DataFile Version") == NULL) {
        printf( "Error : This file not is VTK file [filename=%s].\n", filename);
        return -1;
    }

    // 次行読み込み
    char *ascii = NULL;
    for (n=0; n<2; n++) {
        if (fgets( line, 256, fp ) == NULL) continue;
        trim(line);
        if (strcasecmp(line, "ASCII") == 0) {
            ascii = line;
            break;
        }
    }
    if (ascii == NULL) {
        printf( "Error : This file not is ASCII file [filename=%s].\n", filename);
        return -1;
    }

    // DATASET UNSTRUCTURED_GRID
    if (fgets( line, 256, fp ) == NULL) {
        printf( "Error : can not read \"DATASET UNSTRUCTURED_GRID\" [filename=%s].\n", filename);
        return -1;
    }
    trim(line);
    count = split_string(line, split_list);
    char *dataset = NULL;
    if (count == 2) {
        if (strcasecmp(split_list[0], "DATASET") == 0
            && strcasecmp(split_list[1], "UNSTRUCTURED_GRID") == 0) {
            dataset = split_list[1];
        }
    }
    if (dataset == NULL) {
        printf( "Error : DATASET must be UNSTRUCTURED_GRID.[filename=%s].\n", filename);
        return -1;
    }

    // POINTS, CELLS
    vtk_info->coords = NULL;
    vtk_info->vtk_cells = NULL;
    while( fgets( line, 256, fp ) != NULL ){
        trim(line);
        count = split_string(line, split_list);
        // POINTS
        if (strcasecmp(split_list[0], "POINTS") == 0) {
            if (count <= 1) {
                printf( "Error : not set number of points.[filename=%s].\n", filename);
                return -1;
            }
            if (count >= 3) {
                if (strcasecmp(split_list[2], "float") != 0) {
                    printf( "Warning : points data type is float.[filename=%s].\n", filename);
                }
            }
            int num_points = atoi(split_list[1]);
            if (num_points <= 0) {
                printf( "Error : number of points is zero.[filename=%s].\n", filename);
                return -1;
            }
            vtk_info->coords = (float (*)[3])malloc(sizeof(float)*num_points*3);
            count = read_points(vtk_info->coords, num_points, fp);
            if (count != num_points) {
                free(vtk_info->coords);
                printf( "Error : can not read points.[filename=%s].\n", filename);
                return -1;
            }
            vtk_info->num_points = num_points;
        }

        // CELLS
        if (strcasecmp(split_list[0], "CELLS") == 0) {
            if (count <= 1) {
                printf( "Error : not set number of cells.[filename=%s].\n", filename);
                return -1;
            }
            int num_cells = atoi(split_list[1]);
            if (num_cells <= 0) {
                printf( "Error : number of cells is zero.[filename=%s].\n", filename);
                return -1;
            }
            vtk_info->vtk_cells = (struct vtk_cell_t*)malloc(sizeof(struct vtk_cell_t)*num_cells);
            count = read_cells(vtk_info->vtk_cells, num_cells, fp);
            if (count != num_cells) {
                free(vtk_info->vtk_cells);
                printf( "Error : can not read cells.[filename=%s].\n", filename);
                return -1;
            }
            vtk_info->num_cells = num_cells;
        }

        // CELL_TYPES
        if (strcasecmp(split_list[0], "CELL_TYPES") == 0) {
            if (count <= 1) {
                printf( "Error : not set number of cell_types.[filename=%s].\n", filename);
                return -1;
            }
            int num_cells = atoi(split_list[1]);
            if (vtk_info->vtk_cells == NULL) {
                vtk_info->vtk_cells = (struct vtk_cell_t*)malloc(sizeof(struct vtk_cell_t)*num_cells);
            }
            else {
                if (vtk_info->num_cells != num_cells) {
                    printf( "Error : not equals number of cell_types and cells.[filename=%s].\n", filename);
                    return -1;
                }
            }
            count = read_cell_types(vtk_info->vtk_cells, num_cells, fp);
            if (count != num_cells) {
                free(vtk_info->vtk_cells);
                printf( "Error : can not read cell_types.[filename=%s].\n", filename);
                return -1;
            }
            vtk_info->num_cells = num_cells;
        }
    }

    fclose( fp );
    return 0;
}

/**
 * VTK:CELL_TYPEをUdmElementType_tに変換する.
 * @param vtk_celltype        VTK:CELL_TYPE
 * @return        UdmElementType_t
 */
UdmElementType_t vtkcelltype_to_udmelemettype(int vtk_celltype)
{
    if (vtk_celltype == 10) return Udm_TETRA_4;
    else if (vtk_celltype == 14) return Udm_PYRA_5;
    else if (vtk_celltype == 13) return Udm_PENTA_6;
    else if (vtk_celltype == 11) return Udm_HEXA_8;
    else if (vtk_celltype == 12) return Udm_HEXA_8;
    else if (vtk_celltype == 2) return Udm_NODE;
    else if (vtk_celltype == 3) return Udm_BAR_2;
    else if (vtk_celltype == 5) return Udm_TRI_3;
    else if (vtk_celltype == 8) return Udm_QUAD_4;

    return Udm_ElementTypeUnknown;
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
    int i, j, k;
    int n, m;
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    const char usage[] = "usage: mpiexec -np [PARTATION_SIZE] vtk2cgns [INPUT_VTK_FILE].";

    printf("Start : vtk2cgns\n");

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc <= 1) {
        printf("Error : Please input VTK file.\n");
        printf("%s\n", usage);
        MPI_Finalize();
        return -1;
    }

    // モデルの生成
    UdmHanler_t model = udm_create_model();
    int zone_id = udm_create_zone(model);
    if (zone_id == 0) {
        printf("Error : zone_id is zero.\n");
        udm_delete_model(model);
        return 1;
    }

    // 読み込みVTKファイル
    char *vtk_filename = argv[1];

    // ファイル名, 拡張子分解
    udm_splitpath(vtk_filename, drive, folder, name, ext);

    if (myrank == 0) {
        // VTKファイル読み込み
        printf("Start :: reading VTK!\n");
        struct vtk_t vtk_info;
        if (read_vtk(vtk_filename, &vtk_info) != 0) {
            printf("Error : can not read VTK file.[vtk file=%s]\n", vtk_filename);
            udm_delete_model(model);
            MPI_Finalize();
            return -1;
        }
        if (vtk_info.num_points <= 0 || vtk_info.num_cells <= 0) {
            printf("Error : can not read VTK points or cells.[vtk file=%s]\n", vtk_filename);
            udm_delete_model(model);
            MPI_Finalize();
            return -1;
        }
        printf("End :: reading VTK!\n");

        printf("Start :: create model!\n");

        // 座標の設定
        for (n=0; n<vtk_info.num_points; n++) {
            float x = vtk_info.coords[n][0];
            float y = vtk_info.coords[n][1];
            float z = vtk_info.coords[n][2];
            udm_insert_gridcoordinates(model, zone_id, x, y, z);
        }

        // 要素タイプの取得
        UdmElementType_t element_type = Udm_ElementTypeUnknown;
        for (n=0; n<vtk_info.num_cells; n++) {
            UdmElementType_t vtk_type = vtkcelltype_to_udmelemettype(vtk_info.vtk_cells[n].cell_type);
            if (vtk_type == Udm_ElementTypeUnknown) {
                printf("Error : invalid VTK:CELL_TYPE(=%d).[vtk file=%s]\n", vtk_info.vtk_cells[n].cell_type, vtk_filename);
                udm_delete_model(model);
                MPI_Finalize();
                return -1;
            }
            if (element_type == Udm_ElementTypeUnknown) {
                element_type = vtk_type;
            }
            else if (element_type != vtk_type && element_type != Udm_MIXED) {
                element_type = Udm_MIXED;
            }
        }

        // 要素の追加
        int element_id = udm_create_section(model, zone_id, element_type);
        for (n=0; n<vtk_info.num_cells; n++) {
            // 要素の追加
            UdmSize_t *elem_nodes = (UdmSize_t*)malloc(sizeof(UdmSize_t)*vtk_info.vtk_cells[n].num_cell_points);
            for (m=0; m<vtk_info.vtk_cells[n].num_cell_points; m++) {
                // VTKのPOINTSインデックスは０から、CGNSのノードインデックスは１からであるので＋１する。
                elem_nodes[m] = vtk_info.vtk_cells[n].cell_points[m] + 1;
            }
            UdmElementType_t cell_type = vtkcelltype_to_udmelemettype(vtk_info.vtk_cells[n].cell_type);
            udm_insert_cellconnectivity(model, zone_id, cell_type, elem_nodes);
            free(elem_nodes);
        }

        printf("End :: create model!\n");
    }

    // 部品要素、仮想セル、モデル基本情報のブロードキャスト
    ret = udm_rebuild_model(model);
    if (ret != UDM_OK) {
        printf("Error : rebuildModel");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }

    // 出力設定
    udm_config_setfileprefix(model, name);
    udm_config_setoutputpath(model, name);

    // 分割実行
    printf("Start :: Zoltan Partition!\n");
    // 分割パラメータ=ハイパーグラフ[PARTITION], Zoltanデバッグレベル=1
    ret = udm_partition_zone(model, zone_id);
    if (ret == UDM_WARNING_ZOLTAN_NOCHANGE) {
        printf("Waring :: partition not change!\n");
    }
    else if (ret != UDM_OK) {
        printf("Error : partition : myrank=%d\n", myrank);
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: Zoltan Partition!\n");

    // 分割ファイル出力 : step = 0
    printf("Start :: write model!\n");
    ret = udm_write_model(model, 0, 0.0);
    if (ret != UDM_OK) {
        printf("Error : can not write model\n");
        udm_delete_model(model);
        MPI_Finalize();
        return -1;
    }
    printf("End :: write model!\n");

    // 非構造格子モデル破棄
    udm_delete_model(model);
    MPI_Finalize();

    printf("End : vtk2cgns. \n");

    return 0;
}

