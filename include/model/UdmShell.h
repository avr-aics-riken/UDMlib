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

#ifndef _UDMSHELL_H_
#define _UDMSHELL_H_

/**
 * @file UdmShell.h
 * Shell(2D)要素クラスのヘッダーファイル
 */

#include "model/UdmCell.h"

namespace udm
{

/**
 * Shell(2D)要素クラス
 */
class UdmShell: public UdmCell
{
public:
    UdmShell();
    UdmShell(UdmElementType_t element_type);
    virtual ~UdmShell();
    UdmError_t createComponentCells();
    int getDimensions() const;

private:
    void initialize();
    UdmSize_t createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[]);
};

} /* namespace udm */

#endif /* _UDMSHELL_H_ */
