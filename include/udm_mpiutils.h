/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */


/**
 * @file udm_mpiutils.h
 * MPI関数群のファイル
 */

#ifndef _UDM_MPIUTILS_H_
#define _UDM_MPIUTILS_H_

#ifndef WITHOUT_MPI
#include <mpi.h>
#else
#define MPI_SUCCESS          0      /* Successful return code */
/* Communication argument parameters */
#define MPI_ERR_BUFFER       1      /* Invalid buffer pointer */
#define MPI_ERR_COUNT        2      /* Invalid count argument */
#define MPI_ERR_TYPE         3      /* Invalid datatype argument */
#define MPI_ERR_TAG          4      /* Invalid tag argument */
#define MPI_ERR_COMM         5      /* Invalid communicator */
#define MPI_ERR_RANK         6      /* Invalid rank */
#define MPI_ERR_ROOT         7      /* Invalid root */

/* Define some null objects */
#define MPI_COMM_NULL      ((MPI_Comm)0x04000000)
#define MPI_OP_NULL        ((MPI_Op)0x18000000)
#define MPI_GROUP_NULL     ((MPI_Group)0x08000000)
#define MPI_DATATYPE_NULL  ((MPI_Datatype)0x0c000000)
#define MPI_REQUEST_NULL   ((MPI_Request)0x2c000000)
#define MPI_ERRHANDLER_NULL ((MPI_Errhandler)0x14000000)
#define MPI_MESSAGE_NULL   ((MPI_Message)MPI_REQUEST_NULL)
#define MPI_MESSAGE_NO_PROC ((MPI_Message)0x6c000000)

typedef int MPI_Datatype;
#define MPI_CHAR           ((MPI_Datatype)0x4c000101)
#define MPI_SIGNED_CHAR    ((MPI_Datatype)0x4c000118)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)0x4c000102)
#define MPI_BYTE           ((MPI_Datatype)0x4c00010d)
#define MPI_WCHAR          ((MPI_Datatype)0x4c00040e)
#define MPI_SHORT          ((MPI_Datatype)0x4c000203)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)0x4c000204)
#define MPI_INT            ((MPI_Datatype)0x4c000405)
#define MPI_UNSIGNED       ((MPI_Datatype)0x4c000406)
#define MPI_LONG           ((MPI_Datatype)0x4c000807)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)0x4c000808)
#define MPI_FLOAT          ((MPI_Datatype)0x4c00040a)
#define MPI_DOUBLE         ((MPI_Datatype)0x4c00080b)
#define MPI_LONG_DOUBLE    ((MPI_Datatype)0x4c00100c)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)0x4c000809)
#define MPI_UNSIGNED_LONG_LONG ((MPI_Datatype)0x4c000819)
#define MPI_LONG_LONG      MPI_LONG_LONG_INT

typedef int MPI_Comm;
#define MPI_COMM_WORLD ((MPI_Comm)0x44000000)
#define MPI_COMM_SELF  ((MPI_Comm)0x44000001)
/* Groups */
typedef int MPI_Group;
#define MPI_GROUP_EMPTY ((MPI_Group)0x48000000)

typedef struct MPI_Status {
    int count_lo;
    int count_hi_and_cancelled;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;

/* MPI request opjects */
typedef int MPI_Request;

/* Collective operations */
typedef int MPI_Op;

#define MPI_MAX     (MPI_Op)(0x58000001)
#define MPI_MIN     (MPI_Op)(0x58000002)
#define MPI_SUM     (MPI_Op)(0x58000003)
#define MPI_PROD    (MPI_Op)(0x58000004)
#define MPI_LAND    (MPI_Op)(0x58000005)
#define MPI_BAND    (MPI_Op)(0x58000006)
#define MPI_LOR     (MPI_Op)(0x58000007)
#define MPI_BOR     (MPI_Op)(0x58000008)
#define MPI_LXOR    (MPI_Op)(0x58000009)
#define MPI_BXOR    (MPI_Op)(0x5800000a)
#define MPI_MINLOC  (MPI_Op)(0x5800000b)
#define MPI_MAXLOC  (MPI_Op)(0x5800000c)
#define MPI_REPLACE (MPI_Op)(0x5800000d)
#define MPI_NO_OP   (MPI_Op)(0x5800000e)

#endif

#include <stdio.h>
#include <stdlib.h>

#include "udm_define.h"
#include "udm_errorno.h"

#ifdef __cplusplus
namespace udm {
#endif

/**
 * MPI初期化を行う.
 * @param argc            起動引数の数
 * @param argv            起動引数の文字列
 * @return                成否
 */
inline int udm_mpi_init(int *argc, char ***argv)
{
#ifndef WITHOUT_MPI
    return MPI_Init(argc, argv);
#else
    return MPI_SUCCESS;
#endif
}

/**
 * MPI破棄処理を行う.
 * @return                成否
 */
inline int udm_mpi_finalize(void)
{
#ifndef WITHOUT_MPI
    return MPI_Finalize();
#else
    return MPI_SUCCESS;
#endif
}

/**
 * MPI初期化済みであるかチェックする.
 * @param [out] flag        true:MPI初期化済み
 * @return
 */
inline int udm_mpi_initialized(int *flag)
{
#ifndef WITHOUT_MPI
    return MPI_Initialized(flag);
#else
    *flag = 1;
    return MPI_SUCCESS;
#endif
}

/**
 * MPIランク番号を取得する.
 * @param [in]  comm        MPIコミュニケータ
 * @param [out] rank        MPIランク番号
 * @return        成否
 */
inline int udm_mpi_comm_rank(MPI_Comm comm, int *rank)
{
#ifndef WITHOUT_MPI
    return MPI_Comm_rank(comm, rank);
#else
    *rank = 0;
    return MPI_SUCCESS;
#endif
}

/**
 * MPIプロセス数を取得する.
 * @param [in]  comm        MPIコミュニケータ
 * @param [out] size        MPIプロセス数
 * @return        成否
 */
inline  int udm_mpi_comm_size(MPI_Comm comm, int *size)
{
#ifndef WITHOUT_MPI
    return MPI_Comm_size(comm, size);
#else
    *size = 1;
    return MPI_SUCCESS;
#endif
}

/**
 * UdmSize_tのMPIデータ型を取得する.
 * @param [in] type            データ型
 * @param [out] size            データサイズ
 * @return        UdmSize_tのMPIデータ型
 */
inline int udm_mpi_type_size(MPI_Datatype type, int *size)
{

    if (type == MPI_CHAR) *size = sizeof(char);
    else if (type == MPI_SIGNED_CHAR) *size = sizeof(char);
    else if (type == MPI_UNSIGNED_CHAR) *size = sizeof(char);
    else if (type == MPI_BYTE) *size = sizeof(char);
    else if (type == MPI_WCHAR) *size = sizeof(wchar_t);
    else if (type == MPI_SHORT) *size = sizeof(short);
    else if (type == MPI_UNSIGNED_SHORT) *size = sizeof(short);
    else if (type == MPI_INT) *size = sizeof(int);
    else if (type == MPI_UNSIGNED) *size = sizeof(int);
    else if (type == MPI_LONG) *size = sizeof(long);
    else if (type == MPI_UNSIGNED_LONG) *size = sizeof(long);
    else if (type == MPI_FLOAT) *size = sizeof(float);
    else if (type == MPI_DOUBLE) *size = sizeof(double);
    else if (type == MPI_LONG_DOUBLE) *size = sizeof(long double);
    else if (type == MPI_LONG_LONG_INT) *size = sizeof(long long int);
    else if (type == MPI_UNSIGNED_LONG_LONG) *size = sizeof(long long);
    else if (type == MPI_LONG_LONG) *size = sizeof(long long);
    else *size = 0;

    return *size;
}

/**
 * MPIバリアを行う.
 * @param [in]  comm        MPIコミュニケータ
 * @return        成否
 */
inline  int udm_mpi_barrier(MPI_Comm comm)
{
#ifndef WITHOUT_MPI
    return MPI_Barrier(comm);
#else
    return MPI_SUCCESS;
#endif
}


/**
 * 全プロセスにて処理が正常に終了しているかチェックする.
 * @param [in/out] error            エラー番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
inline UdmError_t udm_mpi_ack(UdmError_t *error, MPI_Comm mpi_communicator)
{
#ifndef WITHOUT_MPI
    int mpi_flag;
    if (mpi_communicator == MPI_COMM_NULL) return UDM_ERROR;

    // MPI初期化済みであるかチェックする.
    MPI_Initialized(&mpi_flag);
    if (!mpi_flag) {
        return *error;
    }

    int send_data = (int)(*error);
    int recv_data;
    MPI_Allreduce(&send_data, &recv_data, 1, MPI_INT, MPI_SUM, mpi_communicator);
    if (recv_data != UDM_OK) {
        if (*error == UDM_OK) {
            *error = UDM_ERROR;
        }
        return *error;
    }
    return UDM_OK;

#else
    return *error;
#endif

}


/**
 * UdmSize_tのMPIデータ型を取得する.
 * @return        UdmSize_tのMPIデータ型
 */
inline MPI_Datatype udm_udmSize_to_mpiType()
{
    MPI_Datatype udm_size_datatype;

#ifndef WITHOUT_MPI
    int size_short=0, size_int=0, size_long=0, size_long_long=0;

    MPI_Type_size(MPI_SHORT, &size_short);
    MPI_Type_size(MPI_INT, &size_int);
    MPI_Type_size(MPI_LONG, &size_long);
#ifdef MPI_LONG_LONG
    MPI_Type_size(MPI_LONG_LONG, &size_long_long);
#else
#ifdef MPI_LONG_LONG_INT
    udm_mpi_type_size(MPI_LONG_LONG_INT, &size_long_long);
#endif
#endif

    if (sizeof(UdmSize_t) == size_short){
        udm_size_datatype = MPI_SHORT;
    }
    else if (sizeof(UdmSize_t) == size_int){
        udm_size_datatype = MPI_INT;
    }
    else if (sizeof(UdmSize_t) == size_long){
        udm_size_datatype = MPI_LONG;
    }
    else if (sizeof(UdmSize_t) == size_long_long) {
#ifdef MPI_LONG_LONG
        udm_size_datatype = MPI_LONG_LONG;
#elif defined(MPI_LONG_LONG_INT)
        udm_size_datatype = MPI_LONG_LONG_INT;
#endif
    }

#else        // WITHOUT_MPI
    if (sizeof(UdmSize_t) == sizeof(short)){
        udm_size_datatype = MPI_SHORT;
    }
    else if (sizeof(UdmSize_t) == sizeof(int)){
        udm_size_datatype = MPI_INT;
    }
    else if (sizeof(UdmSize_t) == sizeof(long)){
        udm_size_datatype = MPI_LONG;
    }
    else if (sizeof(UdmSize_t) == sizeof(long long)) {
        udm_size_datatype = MPI_LONG_LONG;
    }
#endif        // WITHOUT_MPI

    return udm_size_datatype;
}



/**
 * MPIブロードキャストを行う.
 * @param [inout] buffer        送受信バッファー
 * @param [in] count            送受信バッファーサイズ
 * @param [in] datatype            送受信データ型
 * @param [in] root                送信ランク番号
 * @param [in]  comm        MPIコミュニケータ
 * @return        成否
 */
inline  int udm_mpi_bcast(void *buffer, int count, MPI_Datatype datatype,
                             int root, MPI_Comm comm)
{
#ifndef WITHOUT_MPI
    return MPI_Bcast(buffer, count, datatype, root, comm);
#else
    return MPI_SUCCESS;
#endif
}


/**
 * MPI_Alltoallを行う.
 * @param sendbuf        送信バッファー
 * @param sendcount        送信バッファーサイズ
 * @param sendtype        送信データ型
 * @param recvbuf        受信バッファー
 * @param recvcount        受信バッファーサイズ
 * @param recvtype        受信データ型
 * @param comm        MPIコミュニケータ
 * @return        成否
 */
inline  int udm_mpi_alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                void *recvbuf, int recvcount,
                                MPI_Datatype recvtype, MPI_Comm comm)
{
#ifndef WITHOUT_MPI
    return MPI_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
#else
    if (sendbuf == NULL) return MPI_ERR_BUFFER;
    if (recvbuf == NULL) return MPI_ERR_BUFFER;
    if (sendcount <= 0) return MPI_ERR_BUFFER;

    int size = sizeof(int);
    udm_mpi_type_size(sendtype, &size);
    memcpy(recvbuf, sendbuf, sendcount*size);
    return MPI_SUCCESS;
#endif

}

/**
 * MPI受信（ノンブロッキング通信）を行う。
 * @param buf        受信するデータ
 * @param count        データの個数
 * @param datatype        データ型
 * @param source        送信元プロセスのランク
 * @param tag            タグ（メッセージ識別番号）
 * @param comm        MPIコミュニケータ
 * @param request        通信リクエスト
 * @return        成否
 */
inline  int udm_mpi_irecv(void *buf, int count, MPI_Datatype datatype, int source,
                             int tag, MPI_Comm comm, MPI_Request *request)
{
#ifndef WITHOUT_MPI
    return MPI_Irecv(buf, count, datatype, source, tag, comm, request);
#else
    return MPI_SUCCESS;
#endif
}

/**
 * MPI送信（ノンブロッキング通信）を行う。
 * @param buf        送信するデータ
 * @param count        データの個数
 * @param datatype        データ型
 * @param dest        送信先プロセスのランク
 * @param tag            タグ（メッセージ識別番号）
 * @param comm        MPIコミュニケータ
 * @param request        通信リクエスト
 * @return        成否
 */
inline  int udm_mpi_isend(void *buf, int count, MPI_Datatype datatype, int dest,
                             int tag, MPI_Comm comm, MPI_Request *request)
{
#ifndef WITHOUT_MPI
    return MPI_Isend(buf, count, datatype, dest, tag, comm, request);
#else
    return MPI_SUCCESS;
#endif
}

/**
 * MPI確認（ノンブロッキング通信）を行う.
 * @param count            通信リクエスト数
 * @param array_of_requests            通信リクエスト
 * @param array_of_statuses            送信プロセスの情報
 * @return        成否
 */
inline  int udm_mpi_waitall(int count, MPI_Request *array_of_requests,
                               MPI_Status *array_of_statuses)
{
#ifndef WITHOUT_MPI
    return MPI_Waitall(count, array_of_requests, array_of_statuses);
#else
    return MPI_SUCCESS;
#endif
}

/**
 * MPIグループを取得する.
 * @param comm        MPIコミュニケータ
 * @param group        MPIグループ
 * @return        成否
 */
inline int udm_mpi_comm_group(MPI_Comm comm, MPI_Group *group)
{
#ifndef WITHOUT_MPI
    return MPI_Comm_group(comm, group);
#else
    return MPI_SUCCESS;
#endif
}

/**
 * MPIグループのランク数を取得する
 * @param group        MPIグループ
 * @param size        MPIグループのランク数
 * @return        成否
 */
inline int udm_mpi_group_size(MPI_Group group, int *size)
{
#ifndef WITHOUT_MPI
    return MPI_Group_size(group, size);
#else
    *size = 1;
    return MPI_SUCCESS;
#endif
}

/**
 * MPIレデュースを行う
 * @param sendbuf        送信するデータ
 * @param recvbuf        受信するデータ
 * @param count            データの個数
 * @param datatype        データ型
 * @param op            演算の種類
 * @param root            受信するプロセスのランク
 * @param comm            MPIコミュニケータ
 * @return        成否
 */
inline  int udm_mpi_reduce(void *sendbuf, void *recvbuf, int count,
                              MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
#ifndef WITHOUT_MPI
    return MPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
#else
    if (sendbuf == NULL) return MPI_ERR_BUFFER;
    if (recvbuf == NULL) return MPI_ERR_BUFFER;
    if (count <= 0) return MPI_ERR_BUFFER;

    int size = sizeof(int);
    udm_mpi_type_size(datatype, &size);
    memcpy(recvbuf, sendbuf, count*size);
    return MPI_SUCCESS;
#endif
}

/**
 * MPIオールレデュースを行う
 * @param sendbuf        送信するデータ
 * @param recvbuf        受信するデータ
 * @param count            データの個数
 * @param datatype        データ型
 * @param op            演算の種類
 * @param comm            MPIコミュニケータ
 * @return        成否
 */
inline int udm_mpi_allreduce(void *sendbuf, void *recvbuf, int count,
                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
#ifndef WITHOUT_MPI
    return MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
#else
    if (sendbuf == NULL) return MPI_ERR_BUFFER;
    if (recvbuf == NULL) return MPI_ERR_BUFFER;
    if (count <= 0) return MPI_ERR_BUFFER;
    int size = sizeof(int);
    udm_mpi_type_size(datatype, &size);
    memcpy(recvbuf, sendbuf, count*size);
    return MPI_SUCCESS;
#endif
}


/**
 * MPIギャザを行う.
 * @param sendbuf        送信するデータ
 * @param sendcount        送信するデータの個数
 * @param sendtype        送信するデータの型
 * @param recvbuf        受信するデータ
 * @param recvcount        受信するデータの個数
 * @param recvtype        受信するデータの型
 * @param root            受信するプロセスのランク
 * @param comm            MPIコミュニケータ
 * @return        成否
 */
inline  int udm_mpi_gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                              void *recvbuf, int recvcount, MPI_Datatype recvtype,
                              int root, MPI_Comm comm)
{
#ifndef WITHOUT_MPI
    return MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
#else
    if (sendbuf == NULL) return MPI_ERR_BUFFER;
    if (recvbuf == NULL) return MPI_ERR_BUFFER;
    if (sendcount <= 0) return MPI_ERR_BUFFER;

    int size = sizeof(int);
    udm_mpi_type_size(sendtype, &size);
    memcpy(recvbuf, sendbuf, sendcount*size);
    return MPI_SUCCESS;
#endif
}



#ifdef __cplusplus
} /* namespace udm */
#endif

#endif /* _UDM_MPIUTILS_H_ */
