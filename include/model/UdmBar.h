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

#ifndef _UDMBAR_H_
#define _UDMBAR_H_

/**
 * @file UdmBar.h
 * Bar(1D)要素クラスのヘッダーファイル
 */

#include "model/UdmCell.h"

namespace udm
{

/**
 * Bar(1D)要素クラス
 */
class UdmBar: public UdmCell
{
public:
    UdmBar();
    UdmBar(UdmElementType_t element_type);
    virtual ~UdmBar();
    UdmError_t createComponentCells();
    int getDimensions() const;

private:
    void initialize();
    UdmSize_t createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[]);
};

} /* namespace udm */

#endif /* _UDMBAR_H_ */
