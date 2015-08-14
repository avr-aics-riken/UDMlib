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

#ifndef _UDMSEARCHTABLE_H_
#define _UDMSEARCHTABLE_H_

/**
 * @file UdmSearchTable.h
 * 検索テーブルクラスのヘッダファイル
 */

#include "model/UdmNode.h"

namespace udm
{

/**
 * 検索テーブルクラス
 */
class UdmSearchTable
{
private:

    /**
     * ランク番号、ID検索テーブル
     */
    UdmGlobalRankidList  search_table;

public:
    UdmSearchTable();
    UdmSearchTable(const std::vector<UdmNode*> nodes);
    virtual ~UdmSearchTable();

    // ランク番号,ID検索テーブル
    void createSearchTable(const std::vector<UdmNode*> nodes);
    void createMpiRankidTable(const std::vector<UdmNode*> nodes);
    void clearSearchTable();
    void addSearchTable(UdmNode* node);
    UdmNode* findMpiRankInfo(int rankno, UdmSize_t localid) const;
    UdmNode* findMpiRankInfo(const UdmNode *node) const;
    void addMpiRankid(UdmNode* node);
};

} /* namespace udm */

#endif /* _UDMSEARCHTABLE_H_ */
