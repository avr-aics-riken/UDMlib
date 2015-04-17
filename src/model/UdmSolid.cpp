/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmSolid.cpp
 * Solid(3D)要素クラスのソースファイル
 */

#include "model/UdmSolid.h"
#include "model/UdmShell.h"
#include "model/UdmZone.h"
#include "model/UdmSections.h"

namespace udm
{

UdmSolid::UdmSolid()
{
    this->initialize();
}

/**
 * コンストラクタ
 * @param   element_type        要素形状タイプ
 */
UdmSolid::UdmSolid(UdmElementType_t element_type) : UdmCell(element_type)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmSolid::~UdmSolid()
{
}

/**
 * 初期化を行う.
 */
void UdmSolid::initialize()
{
}


/**
 * 部品要素の面（シェル）要素を作成する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmSolid::createComponentCells()
{
    UdmError_t error;
    unsigned int  n;

    // 頂点（ノード）数チェック
    if (this->getNumNodes() != UdmGeneral::getNumVertexOfElementType(this->getElementType())) {
        return UDM_ERROR_INVALID_ELEMENTTYPE;
    }

    // 部品要素数
    unsigned int num_faces = UdmGeneral::getNumComponentOfElementType(this->getElementType());
    if (this->getNumComponentCells() == num_faces) {
        // 作成済み
        return UDM_OK;
    }

    // 部品要素をすべて削除する.
    this->clearComponentCells();

    // 面要素を作成する.
    if (this->getElementType() == Udm_TETRA_4) {
        // 面ID |  接続ノードID
        //  F1  |  N1,N3,N2
        //  F2  |  N1,N2,N4
        //  F3  |  N2,N3,N4
        //  F4  |  N3,N1,N4
        unsigned int num_nodes = 3;
        UdmSize_t face_nodes[][3] = {{1,3,2}, {1,2,4}, {2,3,4}, {3,1,4}};
        for (n=0; n<num_faces; n++) {
            if (this->createComponentCell(
                            Udm_TRI_3,
                            num_nodes, &face_nodes[n][0]) == 0) {
                return UDM_ERROR_INVALID_ELEMENTTYPE;
            }
        }
    }
    else if (this->getElementType() == Udm_PYRA_5) {
        // 面ID |  接続ノードID
        //  F1  |  N1,N4,N3,N2
        //  F2  |  N1,N2,N5
        //  F3  |  N2,N3,N5
        //  F4  |  N3,N4,N5
        //  F5  |  N4,N1,N5
        UdmSize_t face_nodes[] = {1,4,3,2,
                                  1,2,5,
                                  2,3,5,
                                  3,4,5,
                                  4,1,5 };
        int pos = 0;
        for (n=0; n<num_faces; n++) {
            if (n==0) {
                if (this->createComponentCell(
                                Udm_QUAD_4,
                                4, &face_nodes[pos]) == 0) {
                    return UDM_ERROR_INVALID_ELEMENTTYPE;
                }
                pos += 4;
            }
            else {
                if (this->createComponentCell(
                                Udm_TRI_3,
                                3, &face_nodes[pos]) == 0) {
                    return UDM_ERROR_INVALID_ELEMENTTYPE;
                }
                pos += 3;
            }
        }
    }
    else if (this->getElementType() == Udm_PENTA_6) {
        // 面ID |  接続ノードID
        //  F1  |  N1,N2,N5,N4
        //  F2  |  N2,N3,N6,N5
        //  F3  |  N3,N1,N4,N6
        //  F4  |  N1,N3,N2
        //  F5  |  N4,N5,N6
        UdmSize_t face_nodes[] = {1,2,5,4,
                                  2,3,6,5,
                                  3,1,4,6,
                                  1,3,2,
                                  4,5,6 };
        int pos = 0;
        for (n=0; n<num_faces; n++) {
            if (n==0 || n==1 || n==2) {
                if (this->createComponentCell(
                                Udm_QUAD_4,
                                4, &face_nodes[pos]) == 0) {
                    return UDM_ERROR_INVALID_ELEMENTTYPE;
                }
                pos += 4;
            }
            else {
                if (this->createComponentCell(
                                Udm_TRI_3,
                                3, &face_nodes[pos]) == 0) {
                    return UDM_ERROR_INVALID_ELEMENTTYPE;
                }
                pos += 3;
            }
        }
    }
    else if (this->getElementType() == Udm_HEXA_8) {
        // 面ID |  接続ノードID
        //  F1  |  N1,N4,N3,N2
        //  F2  |  N1,N2,N6,N5
        //  F3  |  N2,N3,N7,N6
        //  F4  |  N3,N4,N8,N7
        //  F5  |  N1,N5,N8,N4
        //  F6  |  N5,N6,N7,N8
        unsigned int num_nodes = 4;
        UdmSize_t face_nodes[][4] = {{1,4,3,2}, {1,2,6,5}, {2,3,7,6}, {3,4,8,7}, {1,5,8,4}, {5,6,7,8}};
        for (n=0; n<num_faces; n++) {
            if (this->createComponentCell(
                            Udm_QUAD_4,
                            num_nodes, &face_nodes[n][0]) == 0) {
                return UDM_ERROR_INVALID_ELEMENTTYPE;
            }
        }
    }
    else {
        return UDM_ERROR_INVALID_ELEMENTTYPE;
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
UdmSize_t UdmSolid::createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[])
{
    // 追加済み面数をチェックする
    unsigned int num_faces = 0;
    UdmSize_t num_components = this->getNumComponentCells();
    if (this->getElementType() == Udm_TETRA_4) num_faces = 4;
    else if (this->getElementType() == Udm_PYRA_5)  num_faces = 5;
    else if (this->getElementType() == Udm_PENTA_6)  num_faces = 5;
    else if (this->getElementType() == Udm_HEXA_8)  num_faces = 6;
    else {
        return 0;
    }
    if (num_components >= num_faces) {
        return 0;
    }
    return UdmCell::createComponentCell(element_type, num_nodes, face_nodes);
}

/**
 * 要素（セル）の次元数を取得する.
 * @return        次元数 : Solid=3
 */
int UdmSolid::getDimensions() const
{
    return 3;
}

} /* namespace udm */
