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

#ifndef _UDMCONFIGBASE_H_
#define _UDMCONFIGBASE_H_

#include <TextParser.h>
#include "UdmBase.h"

/**
 * @file UdmConfigDefine.h
 * DFIファイルの文字列、定数、列挙体定義のヘッダーファイル
 */
/**
 * @file UdmConfigBase.h
 * DFIファイルの基底入出力クラスのヘッダーファイル
 */

namespace udm {

/**
 * DFI種別列挙体
 */
typedef enum {    Udm_DfiTypeUnknown,
                Udm_UnsDfiType
} UdmDfiType_t;


/**
 * ファイル命名書式列挙体
 */
typedef enum {    Udm_FieldFilenameFormatUnknown = 0,
                Udm_step_rank = 1,
                Udm_rank_step = 2,
                Udm_printf = 3,
                Udm_FieldFilenameFormatDefault = 1
} UdmFieldFilenameFormat_t;

/**
 * DFIファイルの基底入出力クラス
 */
class UdmConfigBase : public UdmBase {
protected:
    /**
     * DFIラベル名
     */
    std::string name;

    /**
     * DFIファイルパーサー
     */
    TextParser *parser;

protected:

    /**
     * 初期化を行う.
     */
    virtual void initialize(void) = 0;

    UdmError_t getValue(const std::string& label,std::string& value);
    UdmError_t getValueList(const std::string& label, std::vector<std::string>& values);
    UdmError_t getChildLabels(const std::string& label, std::vector<std::string>& values);
    int countLabel(const std::string label);
    bool checkLabel(const std::string label, bool fullPath = true);
    bool checkNode(const std::string node);
    int splitLabelIndex(const std::string &label) const;
    UdmError_t getLabels(const std::string& label, std::vector<std::string>& values);
    UdmError_t setValue(const std::string& label, const std::string& value, UdmDataType_t  data_type = Udm_String);
    UdmError_t setValue(const std::string& label, long long value);
    UdmError_t setValue(const std::string& label, UdmSize_t value);
    UdmError_t setValue(const std::string& label, double value);
    UdmError_t setValueList(const std::string& label, std::vector<std::string> values);
    bool isListValue(const std::string &value);
    bool convertBool(const std::string value, UdmError_t *ierror);
    long long convertLong(const std::string& value, UdmError_t *ierror);
    double convertDouble(const std::string& value, UdmError_t *ierror);
    UdmSize_t convertSize_t(const std::string& value, UdmError_t *ierror);
    UdmDataType_t convertDataType(const std::string value, UdmError_t *ierror);
    UdmFileCompositionType_t convertFileCompositionType(const std::string value, UdmError_t *ierror);
    UdmGridLocation_t convertGridLocation(const std::string value, UdmError_t *ierror);
    UdmVectorType_t convertVectorType(const std::string value, UdmError_t *ierror);
    UdmDfiType_t convertDfiType(const std::string value, UdmError_t *ierror);
    UdmFieldFilenameFormat_t convertFieldFilenameFormat(const std::string value, UdmError_t *ierror);
    static inline UdmError_t toString(UdmFileCompositionType_t value, std::string &buf);
    static inline UdmError_t toString(double value, std::string &buf);
    static inline UdmError_t toString(long long value, std::string &buf);
    static inline UdmError_t toString(UdmDfiType_t value, std::string &buf);
    static inline UdmError_t toString(UdmFieldFilenameFormat_t value, std::string &buf);
    static inline UdmError_t toString(UdmOnOff_t value, std::string &buf);
    static inline UdmError_t toString(UdmEnable_t value, std::string &buf);
    static inline UdmError_t toString(UdmDataType_t value, std::string &buf);
    static inline UdmError_t toString(UdmSize_t value, std::string &buf);
    static inline UdmError_t toString(UdmGridLocation_t value, std::string &buf);
    static inline UdmError_t toString(UdmVectorType_t value, std::string &buf);
    static inline UdmError_t toString(bool value, std::string &buf);
    UdmError_t writeLabel(FILE *fp, unsigned int indent, const std::string &label);
    UdmError_t writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, std::string &value);
    UdmError_t writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, int value);
    UdmError_t writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, long long value);
    UdmError_t writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, UdmSize_t value);
    UdmError_t writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, double value);
    UdmError_t writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, std::vector<std::string> values);
    UdmError_t writeText(FILE *fp, unsigned int indent, const std::string &text);
    UdmError_t writeCloseTab(FILE *fp, unsigned int indent);

public:
    /**
     * コンストラクタ
     */
    UdmConfigBase();

    /**
     * コンストラクタ:TextParser
     * @param  parser        TextParser
     */
    UdmConfigBase(TextParser *parser);

    /**
     * コンストラクタ：ラベル名
     * @param name        ラベル名
     */
    UdmConfigBase(std::string name);

    /**
     * デストラクタ
     */
    virtual ~UdmConfigBase();

    /**
     * ラベル名を設定する.
     * @param name        ラベル名
     */
    void setName(std::string name);

    /**
     * ラベル名を取得する.
     * @return        ラベル名
     */
    std::string getName() const;

    TextParser* getTextParser();
    void setTextParser(TextParser* parser);
    virtual UdmError_t getDfiValue(const std::string &label_path, std::string &value) const = 0;
    virtual UdmError_t setDfiValue(const std::string &label_path, const std::string &value) = 0;
    virtual int getNumDfiValue(const std::string &label_path) const = 0;
};


/**
 * FileCompositionTypeを文字列に変換する.
 * @param [in]  value        FileCompositionType
 * @param [out] buf        FileCompositionType文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmFileCompositionType_t value, std::string &buf)
{
    switch (value) {
    case (Udm_IncludeGrid): buf = "IncludeGrid"; return UDM_OK;
    case (Udm_ExcludeGrid): buf = "ExcludeGrid"; return UDM_OK;
    case (Udm_AppendStep): buf = "AppendStep"; return UDM_OK;
    case (Udm_EachStep): buf = "EachStep"; return UDM_OK;
    case (Udm_GridConstant): buf = "GridConstant"; return UDM_OK;
    case (Udm_GridTimeSlice): buf = "GridTimeSlice"; return UDM_OK;
    default : break;
    }

    return UDM_ERROR;
}


/**
 * long long数値を文字列に変換する.
 * @param [in] value        long long数値
 * @param [out] buf            long long文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(long long value, std::string &buf)
{
    char value_buf[std::numeric_limits<long long>::digits10 + 2]; // '-' + NULL
    std::sprintf(value_buf, "%lld", value);
    buf = value_buf;
    return UDM_OK;
}

/**
 * UdmSize_t数値を文字列に変換する.
 * @param [in] value        UdmSize_t数値
 * @param [out] buf            UdmSize_t文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmSize_t value, std::string &buf)
{
    char value_buf[std::numeric_limits<unsigned long long>::digits10 + 2]; // '-' + NULL
    std::sprintf(value_buf, "%lu", (unsigned long long)value);
    buf = value_buf;
    return UDM_OK;
}


/**
 * Double数値を文字列に変換する.
 * @param [in] value        Double数値
 * @param [out] buf            Double文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 *
 */
inline UdmError_t UdmConfigBase::toString(double value, std::string &buf)
{
    char value_buf[std::numeric_limits<double>::max_exponent10
             + 6   // fixed precision (printf's default)
             + 3]; // '-' + '.' + NULL
    std::sprintf(value_buf, "%e", value);
    buf = value_buf;
    return UDM_OK;
}

/**
 * DFI種別:UdmDfiType_tを文字列に変換する.
 * @param value        DfiType
 * @param buf        DfiType文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmDfiType_t value, std::string &buf)
{
    switch (value) {
    case (Udm_UnsDfiType): buf = UDM_DFI_DFITYPE_UNS; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}

/**
 * ファイル命名書式:UdmFieldFilenameFormat_tを文字列に変換する.
 * @param value        UdmFieldFilenameFormat_t
 * @param buf        FieldFilenameFormat文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmFieldFilenameFormat_t value, std::string &buf)
{
    switch (value) {
    case (Udm_step_rank): buf = "step_rank"; return UDM_OK;
    case (Udm_rank_step): buf = "rank_step"; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}

/**
 * ON/OFFタイプ:UdmOnOff_tを文字列に変換する.
 * @param value        UdmOnOff_t
 * @param buf        OnOff文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmOnOff_t value, std::string &buf)
{
    switch (value) {
    case (Udm_off): buf = "off"; return UDM_OK;
    case (Udm_on): buf = "on"; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}

/**
 * Enableタイプ:UdmEnable_tを文字列に変換する.
 * @param value        UdmEnable_t
 * @param buf        OnOff文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmEnable_t value, std::string &buf)
{
    switch (value) {
    case (udm_disable): buf = "disable"; return UDM_OK;
    case (udm_enable): buf = "enable"; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}


/**
 * DataType:UdmDataType_tを文字列に変換する.
 * @param value        UdmDataType_t
 * @param buf        DataType文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmDataType_t value, std::string &buf)
{
    switch (value) {
    case (Udm_Integer): buf = "Integer"; return UDM_OK;
    case (Udm_LongInteger): buf = "LongInteger"; return UDM_OK;
    case (Udm_RealSingle): buf = "RealSingle"; return UDM_OK;
    case (Udm_RealDouble): buf = "RealDouble"; return UDM_OK;
    case (Udm_String): buf = "String"; return UDM_OK;
    case (Udm_Boolean): buf = "Boolean"; return UDM_OK;
    case (Udm_Numeric): buf = "Numeric"; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}

/**
 * GridLocationを文字列に変換する.
 * @param value        UdmGridLocation_t
 * @param buf        GridLocation文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmGridLocation_t value, std::string &buf)
{
    switch (value) {
    case (Udm_Vertex): buf = "Vertex"; return UDM_OK;
    case (Udm_CellCenter): buf = "CellCenter"; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}

/**
 * VectorTypeを文字列に変換する.
 * @param value        UdmVectorType_t
 * @param buf        VectorType文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(UdmVectorType_t value, std::string &buf)
{
    switch (value) {
    case (Udm_Scalar): buf = "Scalar"; return UDM_OK;
    case (Udm_Vector): buf = "Vector"; return UDM_OK;
    default : break;
    }
    return UDM_ERROR;
}

/**
 * bool値を文字列に変換する.
 * @param value        bool値
 * @param buf        true/false文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
inline UdmError_t UdmConfigBase::toString(bool value, std::string &buf)
{
    if (value) {
        buf = "true";
        return UDM_OK;
    }
    else {
        buf = "false";
        return UDM_OK;
    }

    return UDM_ERROR;
}


} /* namespace udm */

#endif /* _UDMCONFIGBASE_H_ */
