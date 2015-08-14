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
 * @file UdmBar.cpp
 * Bar要素クラスのソースファイル
 */

#include "model/UdmBar.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmBar::UdmBar()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param   element_type        要素形状タイプ
 */
UdmBar::UdmBar(UdmElementType_t element_type) : UdmCell(element_type)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmBar::~UdmBar()
{
}

/**
 * 初期化を行う.
 */
void UdmBar::initialize()
{
}

/**
 * 部品要素（セル）を作成する.
 * BAR要素の場合は、部品要素はなし。
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmBar::createComponentCells()
{
    return UDM_OK;
}

/**
 * 部品要素（セル）を生成して、UdmSectionsクラスに格納を行う.
 * @param element_type        要素形状タイプ
 * @param num_nodes            頂点（ノード）数
 * @param face_nodes        面-頂点テーブル
 * @return        部品要素（セル）ID  : 追加失敗、エラーの場合は0を返す。
 */
UdmSize_t UdmBar::createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[])
{
    // BAR要素の場合は、部品要素はなし。
    return 0;
}


/**
 * 要素（セル）の次元数を取得する.
 * @return        次元数 : Bar=1
 */
int UdmBar::getDimensions() const
{
    return 1;
}

} /* namespace udm */
