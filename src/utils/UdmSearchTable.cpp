/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmSearchTable.cpp
 * 検索テーブルクラスのソースファイル
 */

#include "utils/UdmSearchTable.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmSearchTable::UdmSearchTable()
{
    this->search_table.clear();
}

/**
 * コンストラクタ
 * @param nodes        検索対象の節点（ノード）リスト
 */
UdmSearchTable::UdmSearchTable(const std::vector<UdmNode*> nodes)
{
    this->search_table.clear();
    this->createSearchTable(nodes);
}

/**
 * デストラクタ
 */
UdmSearchTable::~UdmSearchTable()
{
    this->search_table.clear();
}

/**
 * ランク番号、ID検索テーブルを作成する.
 * @param nodes        検索対象の節点（ノード）リスト
 */
void UdmSearchTable::createSearchTable(const std::vector<UdmNode*> nodes)
{
    this->clearSearchTable();

    std::vector<UdmNode*>::const_iterator itr;
    for (itr=nodes.begin(); itr!=nodes.end(); itr++) {
        UdmNode *node = *itr;
        this->addSearchTable(node);
    }

    return;
}

/**
 * ランク番号、ID検索テーブルをクリアする.
 */
void UdmSearchTable::clearSearchTable()
{
    this->search_table.clear();
    return;
}

/**
 * ランク番号、ID検索テーブルを作成する.
 * 節点（ノード）のランク番号、ID、以前のランク番号、ID、MPI接続のランク番号、IDの検索テーブルを作成する.
 * @param node        節点（ノード）
 */
void UdmSearchTable::addSearchTable(UdmNode* node)
{

    int n;
    int len;
    int rankno;
    UdmSize_t localid;

    // ランク番号、ID
    this->search_table.addGlobalRankid(node->getMyRankno(), node->getId(), node);

    // 以前のランク番号, ID
    len = node->getNumPreviousRankInfos();
    for (n=1; n<=len; n++) {
        node->getPreviousRankInfo(n, rankno, localid);
        this->search_table.addGlobalRankid(rankno, localid, node);
    }

    // MPI接続ランク番号, ID
    len = node->getNumMpiRankInfos();
    for (n=1; n<=len; n++) {
        node->getMpiRankInfo(n, rankno, localid);
        this->search_table.addGlobalRankid(rankno, localid, node);
    }

    return;
}

/**
 * MPI接続ランク番号, IDのみのランク番号、ID検索テーブルを作成する.
 * @param nodes        検索対象の節点（ノード）リスト
 */
void UdmSearchTable::createMpiRankidTable(const std::vector<UdmNode*> nodes)
{
    this->clearSearchTable();

    std::vector<UdmNode*>::const_iterator itr;
    for (itr=nodes.begin(); itr!=nodes.end(); itr++) {
        UdmNode *node = *itr;
        this->addMpiRankid(node);
    }

    return;
}

/**
 * MPI接続ランク番号, IDのみのランク番号、ID検索テーブルを作成する.
 * @param node        節点（ノード）
 */
void UdmSearchTable::addMpiRankid(UdmNode* node)
{

    int n;
    int len;
    int rankno;
    UdmSize_t localid;

    // MPI接続ランク番号, ID
    len = node->getNumMpiRankInfos();
    for (n=1; n<=len; n++) {
        node->getMpiRankInfo(n, rankno, localid);
        this->search_table.addGlobalRankid(rankno, localid, node);
    }

    return;
}

/**
 * ランク番号、IDの節点（ノード）を検索する.
 * @param rankno        ランク番号
 * @param localid        ID
 * @return            検索結果節点（ノード）
 */
UdmNode* UdmSearchTable::findMpiRankInfo(int rankno, UdmSize_t localid) const
{
    UdmEntity *node = this->search_table.getReferenceEntity(rankno, localid);
    return (UdmNode*)node;
}

/**
 * ランク番号、IDの節点（ノード）を検索する.
 * @param node        検索節点（ノード）
 * @return            検索結果節点（ノード）
 */
UdmNode* UdmSearchTable::findMpiRankInfo(const UdmNode* node) const
{
    if (node == NULL) return NULL;
    if (this->search_table.size() <= 0) return NULL;

    int n;
    int rankno;
    UdmSize_t localid;
    UdmNode *find_node = NULL;

    rankno = node->getMyRankno();
    localid = node->getId();
    find_node = (UdmNode*)this->search_table.getReferenceEntity(rankno, localid);
    if (find_node != NULL) {
        return find_node;
    }

    // 共有節点（ノード）,内部境界情報から検索する.
    int num_rankinfo = node->getNumMpiRankInfos();
    for (n=1; n<=num_rankinfo; n++) {
        node->getMpiRankInfo(n, rankno, localid);
        find_node = this->findMpiRankInfo(rankno, localid);
        if (find_node != NULL) {
            break;
        }
    }

    return find_node;
}

} /* namespace udm */
