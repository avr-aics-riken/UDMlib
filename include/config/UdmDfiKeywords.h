/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMCONFIGDEFINE_H_
#define _UDMCONFIGDEFINE_H_


namespace udm {

/**
 * ON/OFFタイプ文字列
 * enum UdmOnOff_tと同一定義順にすること。
 *
 */
const char *UdmOnOff_s[] = { "off", "on" };

/**
 * ENABLEタイプ文字列
 * enum UdmEnable_tと同一定義順にすること。
 */
const char *UdmEnable_s[] = { "disable", "enable" };

/**
 * データ型文字列
 * enum UdmDataType_tと同一定義順にすること。
 */
const char *UdmDataType_s[] = {
                "Unknown",
                "Integer", "LongInteger",
                "RealSingle", "RealDouble", "Real",
                "String"
};

/**
 * DIF/FileInfo/FieldFilenameFormat:ファイル命名書式
 * enum UdmFieldFilenameFormat_tと同一定義順にすること。
 */
const char *UdmFieldFilenameFormat_s[] = {
                "unknown",
                "step_rank", "rank_step", "printf"};


/**
 * CGNSファイル構成タイプ文字列
 * enum UdmFileCompositionType_tと同一定義順にすること。
 */
const char *UdmFileCompositionType_s[] = {
                "Unknown",
                "IncludeGrid", "ExcludeGrid",
                "AppendStep", "EachStep",
                "CreateLink", "WithLink"
};

/**
 * CGNS物理量適用位置文字列
 * enum UdmGridLocation_tと同一定義順にすること。
 */
const char *UdmGridLocation_s[] = {
                "Unknown",
                "Vertex", "CellCenter"
};

/**
 * ベクトルデータタイプ文字列
 * enum UdmVectorType_tと同一定義順にすること。
 */
const char *UdmVectorType_s[] = {
                "Unknown",
                "Scalar", "Vector"
};

} /* namespace udm */

#endif /* _UDMCONFIGDEFINE_H_ */
