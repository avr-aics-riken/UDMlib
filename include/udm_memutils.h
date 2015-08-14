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
 * @file udm_memutils.h
 * メモリ操作関数群のファイル
 */

#ifndef _UDM_MEMUTILS_H_
#define _UDM_MEMUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/resource.h>

#ifdef __cplusplus
namespace udm {
#endif

/**
 * 設定ID配列にID,ランク番号を設定する.
 * 設定IDサイズ:1 = IDのみ
 * 設定IDサイズ:2 = ID + ランク番号
 * 設定IDサイズ:3 = ID:上位4バイト + ID:下位4バイト + ランク番号
 * @param ids        設定ID配列
 * @param id        ID
 * @param rankno    ランク番号
 * @param num_entries    設定IDサイズ
 * @return        設定ID配列
 */
template <class DATA_TYPE>
inline int udm_set_entryid(DATA_TYPE *ids, UdmSize_t id, int rankno, int num_entries)
{
    int n = 0;
    if (num_entries == 1) {
        ids[n++] = (DATA_TYPE)id;
    }
    else if (num_entries == 2) {
        ids[n++] = (DATA_TYPE)id;
        ids[n++] = rankno;
    }
    else if (num_entries == 3) {
        if (sizeof(DATA_TYPE) >= sizeof(UdmSize_t)) {
            ids[n++] = (DATA_TYPE)id;
            ids[n++] = 0;
            ids[n++] = rankno;
        }
        else if (sizeof(DATA_TYPE) < sizeof(UdmSize_t)) {
#ifdef UDM_SIZE64
            ids[n++] = (DATA_TYPE)(id >> 32);
            ids[n++] = (DATA_TYPE)(id & 0xFFFFFFFF);
            ids[n++] = rankno;
#else
            return 0;
#endif
        }
    }
    else {
        return 0;
    }

    return num_entries;
}

/**
 * 設定ID配列からID,ランク番号を取得する.
 * 設定IDサイズ:1 = IDのみ（ランク番号=-1)
 * 設定IDサイズ:2 = ID + ランク番号
 * 設定IDサイズ:3 = ID(=上位4バイト+下位4バイト) + ランク番号
 * @param [in]  ids        設定ID配列
 * @param [out] id        ID
 * @param [out] rankno    ランク番号
 * @param [in] num_entries    設定IDサイズ
 * @return        設定ID配列
 */
template <class DATA_TYPE>
inline int udm_get_entryid(DATA_TYPE *ids, UdmSize_t &id, int &rankno, int num_entries)
{
    if (num_entries == 1) {
        id = (UdmSize_t)ids[0];
        rankno = -1;
    }
    else if (num_entries == 2) {
        id = (UdmSize_t)ids[0];
        rankno = ids[1];
    }
    else if (num_entries == 3) {
        if (sizeof(DATA_TYPE) >= sizeof(UdmSize_t)) {
            id = ids[0];
            rankno = ids[2];
        }
        else if (sizeof(DATA_TYPE) < sizeof(UdmSize_t)) {
#ifdef UDM_SIZE64
            id = ((UdmSize_t)ids[0] << 32) | ids[1];
            rankno = ids[2];
#else
            return 0;
#endif
        }
    }
    else {
        return 0;
    }

    return num_entries;
}

/**
 * 総数を分割数で振り分ける。
 * num_total=9,num_div=2,div_id=1,[out]num_ids={5,6,7,8},return=4
 * @param [in]  num_total        総数
 * @param [in] num_div        分割数
 * @param [in] div_id        取得インデックス（０～）
 * @param [out] num_ids        振分結果
 * @return            振分数
 */
inline int udm_divide_serialids(int num_total, int num_div, int div_id, int *num_ids)
{
    int i;
    int count = 0;

    if (num_total == 0) return 0;
    if (num_div == 0) return 0;
    if (num_ids == NULL) return 0;

    // 1つ当りの振分余り
    int rest_rank = num_total % num_div;
    // 1つ当りの振分数 ：切り上げ
    int par_rank = (num_total + (num_div - 1) )/num_div;
    if (div_id < rest_rank || rest_rank == 0) {
        for (i=div_id*par_rank; i<(div_id+1)*par_rank; i++) {
            if (i<num_total) {
                num_ids[count++] = i;
            }
        }
    }
    else {
        int start = rest_rank*par_rank + (div_id-rest_rank)*(par_rank-1);
        for (i=0; i<par_rank-1; i++) {
            if ( (i+start)<num_total) {
                num_ids[count++] = i+start;
            }
        }
    }

    return count;
}


/**
 * 総数を分割数で振り分ける。
 * num_total=9,num_div=2,div_id=1,[out]num_ids={1,3,5,7},return=4
 * @param [in]  num_total        総数
 * @param [in] num_div        分割数
 * @param [in] div_id        取得インデックス（０～）
 * @param [out] num_ids        振分結果
 * @return            振分数
 */
inline int udm_divide_parallelids(int num_total, int num_div, int div_id, int *num_ids)
{
    int i;
    int count = 0;

    if (num_total == 0) return 0;
    if (num_div == 0) return 0;
    if (num_ids == NULL) return 0;

    // 1つ当りの振分余り
    // int rest_rank = num_total % num_div;
    // 1つ当りの振分数 ：切り上げ
    int par_rank = (num_total + (num_div - 1) )/num_div;
    for (i=0; i<par_rank; i++) {
        if (i*num_div+div_id<num_total) {
            num_ids[count++] = i*num_div+div_id;
        }
    }

    return count;
}

/**
 * プロセスの使用メモリを文字列で取得する.
 * /proc/[PID]/statusからVmSizeを取得する.
 * @param [out]  mem_output   使用メモリ(文字列:128以上の領域を確保していること)
 */
inline void udm_process_memsize(size_t &mem_vsz, size_t &mem_rss)
{
    struct rusage ru;
    int result = 0;

    mem_vsz = 0;
    mem_rss = 0;
    result = getrusage(RUSAGE_SELF, &ru);
    if(result) {
        return;
    }
    mem_rss = ru.ru_maxrss;

    long statm_rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL ) return;
    if ( fscanf( fp, "%*s%ld", &statm_rss ) != 1 ) {
        fclose( fp );
        return;
    }
    fclose( fp );
    mem_rss = (size_t)statm_rss * (size_t)sysconf( _SC_PAGESIZE);

    return;
}


#ifdef __cplusplus
} /* namespace udm */
#endif

#endif /* _UDM_MEMUTILS_H_ */
