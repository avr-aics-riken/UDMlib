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
 * @file UdmShell.cpp
 * Shell(2D)要素クラスのソースファイル
 */

#include "model/UdmShell.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmShell::UdmShell()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param   element_type        要素形状タイプ
 */
UdmShell::UdmShell(UdmElementType_t element_type) : UdmCell(element_type)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmShell::~UdmShell()
{
    this->initialize();
}

/**
 * 初期化を行う.
 */
void UdmShell::initialize()
{
}

/**
 * 部品要素（セル）を作成する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmShell::createComponentCells()
{
    UdmError_t error;
    unsigned int n;

    // 頂点（ノード）数チェック
    if (this->getNumNodes() != UdmGeneral::getNumVertexOfElementType(this->getElementType())) {
        return UDM_ERROR_INVALID_ELEMENTTYPE;
    }
    // 部品要素数
    unsigned int num_bars = UdmGeneral::getNumComponentOfElementType(this->getElementType());
    if (this->getNumComponentCells() == num_bars) {
        // 作成済み
        return UDM_OK;
    }

    // 部品要素をすべて削除する.
    this->clearComponentCells();

    // BAR要素を作成する.
    if (this->getElementType() == Udm_TRI_3) {
        // エッジID | 接続ノードID
        //    E1   |   N1, N2
        //    E2   |   N2, N3
        //    E3   |   N3, N1
        unsigned int num_nodes = 2;
        UdmSize_t bar_nodes[][2] = {{1,2}, {2,3}, {3,1}};
        for (n=0; n<num_bars; n++) {
            if (this->createComponentCell(
                            Udm_BAR_2,
                            num_nodes, &bar_nodes[n][0]) == 0) {
                return UDM_ERROR_INVALID_ELEMENTTYPE;
            }
        }
    }
    else if (this->getElementType() == Udm_QUAD_4) {
        // エッジID | 接続ノードID
        //    E1   |   N1,N2
        //    E2   |   N2, N3
        //    E3   |   N3, N4
        //    E4   |   N4, N1
        unsigned int num_nodes = 2;
        UdmSize_t bar_nodes[][2] = {{1,2}, {2,3}, {3,4}, {4, 1} };
        for (n=0; n<num_bars; n++) {
            if (this->createComponentCell(
                            Udm_BAR_2,
                            num_nodes, &bar_nodes[n][0]) == 0) {
                return UDM_ERROR_INVALID_ELEMENTTYPE;
            }
        }
    }

    return UDM_OK;
}


/**
 * 部品要素（セル）を生成して、UdmSectionsクラスに格納を行う.
 * @param element_type        要素形状タイプ
 * @param num_nodes            頂点（ノード）数
 * @param face_nodes        面-頂点テーブル
 * @return        部品要素（セル）ID  : 追加失敗、エラーの場合は0を返す。
 */
UdmSize_t UdmShell::createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[])
{
    // 追加済みBAR数をチェックする
    unsigned int num_bars = 0;
    UdmSize_t num_components = this->getNumComponentCells();
    if (this->getElementType() == Udm_TRI_3) num_bars = 3;
    else if (this->getElementType() == Udm_QUAD_4)  num_bars = 4;
    else {
        return 0;
    }
    if (num_components >= num_bars) {
        return 0;
    }
    return UdmCell::createComponentCell(element_type, num_nodes, face_nodes);
}


/**
 * 要素（セル）の次元数を取得する.
 * @return        次元数 : Shell=2
 */
int UdmShell::getDimensions() const
{
    return 2;
}

} /* namespace udm */
