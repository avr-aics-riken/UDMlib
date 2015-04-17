/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmConfigBase.cpp
 * DFIファイルの基底入出力クラスのヘッダーファイル
 */

#include "config/UdmConfigBase.h"
#include "config/UdmDfiKeywords.h"
#include "udm_strutils.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmConfigBase::UdmConfigBase() : parser(NULL)
{
}

/**
 * コンストラクタ:TextParser
 * @param  parser        TextParser
 */
UdmConfigBase::UdmConfigBase(TextParser *parser) : parser(parser)
{

}


/**
 * コンストラクタ：ラベル名
 * @param name        ラベル名
 */
UdmConfigBase::UdmConfigBase(std::string name) : parser(NULL)
{
    this->setName(name);
}

/**
 * デストラクタ
 */
UdmConfigBase::~UdmConfigBase()
{
}

/**
 * DFIパーサを取得する.
 * @return        DFIパーサ
 */
TextParser* UdmConfigBase::getTextParser()
{
    return this->parser;
}

/**
 * DFIパーサを設定する.
 * @param parser        DFIパーサ
 */
void UdmConfigBase::setTextParser(TextParser* parser)
{
    this->parser = parser;
}


/**
 * ラベル名を設定する.
 * @param name        ラベル名
 */
void UdmConfigBase::setName(std::string name)
{
    this->name = name;
};

/**
 * ラベル名を取得する.
 * @return        ラベル名
 */
std::string UdmConfigBase::getName() const
{
    return this->name;
};

/**
 * ラベルのDFIファイル内の値を取得する
 * @param [in]  label        ラベル名
 * @param [out] value        値
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::getValue(const std::string& label,std::string& value)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }
    if (!this->checkLabel(label)) {
        return UDM_ERROR;
    }
    if (this->parser->getValue(label, value ) != TP_NO_ERROR ) {
        return UDM_ERROR;
    }

    return UDM_OK;
}

/**
 * ラベルのDFIファイル内のリストを取得する。
 * @param [in]  label        ラベル名
 * @param [out] values    リスト
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::getValueList(const std::string& label, std::vector<std::string>& values)
{
    int ierror = UDM_OK, count = 0, i;
    std::string value, node;
    std::vector<std::string> node_values;

    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }
    count = this->countLabel(label);
    if (count <= 0) {
        return UDM_ERROR;
    }
    for (i=0; i<count; i++) {
        ierror = UDM_OK;
        std::ostringstream buf;
        buf << label << "[" << i << "]";
        node = buf.str();
        if (!this->checkLabel(node)) {
            ierror = UDM_ERROR;
            if (count == 1) {
                node = label;
                if (this->checkLabel(node)) {
                    ierror = UDM_OK;
                }
            }
        }
        if (ierror != UDM_OK) {
            return UDM_ERROR;
        }

        if ( this->getValue(node, value ) == UDM_OK ) {
            TextParserValueType type = this->parser->getType(node, &ierror);
            if (type == TP_STRING_VALUE) {
                values.push_back(value);
            }
            else {
                if (this->parser->splitVector(value, node_values) == TP_NO_ERROR) {
                    values.insert(values.end(), node_values.begin(), node_values.end());
                }
                else {
                    return UDM_ERROR;
                }
            }
        }
        else {
            return UDM_ERROR;
        }
    }

    return UDM_OK;
}

/**
 * DFIファイルからラベルの子要素リストを取得する.
 * ラベルのパス名と一致する子要素名を取得する.
 * 但し、孫要素を持つラベルは取得しない.
 * 例 :.
 *         label = /UnitList
 *         /UnitList/Pressure        取得
 *         /UnitList/Pressure[0]    取得
 *         /UnitList/Pressure/Unit 取得しない
 *
 * @param label            ラベル名
 * @param values        取得ラベルリスト
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::getChildLabels(const std::string& label, std::vector<std::string>& values)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }
    int ierror;
    unsigned int len=label.length();
    int inode=0;
    int next=0;
    int i;
    std::string node, child_label;
    std::vector<std::string> labels;

    // Nodeがあるかチェック
    ierror = this->getLabels(label, labels);
    if (ierror != TP_NO_ERROR) {
        return UDM_ERROR;
    }

    for (i = 0; i < labels.size(); i++) {
        if (labels[i].length() <= len) continue;

        node = labels[i].substr(0, len);
        if ( !strcasecmp(node.c_str(), label.c_str()) ) {
            next = labels[i].find("/", len);
            if ( next == 0 ) continue;        // 子要素を持たない。
            if (labels[i].length() <= next+1) continue;    // 子要素を持たない。
            next = labels[i].find("/", next+1);
            if ( next == 0 ) {
                child_label = labels[i];
            }
            else {
                child_label = labels[i].substr(0, next);
            }
            // 追加済みかチェックする.
            std::vector< std::string >::iterator itr = std::find( values.begin(), values.end() , child_label );
            if( itr != values.end() ) continue;
            values.push_back(child_label);
        }
    }

    return UDM_OK;
}


/**
 * ラベルがDFIファイルに存在するかチェックする.
 * ラベルは添字(="[X]")まで一致しているかチェックする。
 * @param label            ラベル名
 * @param fullPath        フルパスチェック
 * @return    true=存在する
 */
bool UdmConfigBase::checkLabel(const std::string label, bool fullPath)
{
    TextParserError e_error;
    std::string value;
    size_t i;
    int ierror;

    if (!this->parser) {
        return false;
    }

    // ラベルがあるかチェック
    std::vector<std::string> labels;
    if (fullPath) {
        ierror = this->getLabels(label, labels);
        if (ierror != UDM_OK) {
            return false;
        }
    }
    else {
        e_error = this->parser->getLabels(labels);
        if (e_error != TP_NO_ERROR) {
            return false;
        }
    }

    int flag = 0;
    for (i = 0; i < labels.size(); i++) {
        if (!strcasecmp(label.c_str(), labels[i].c_str())) {
            flag = 1;
            break;
        }
    }

    if (flag == 0) {
        return false;
    }

    return true;
}


/**
 * ノードがDFIファイルに存在するかチェックする.
 * @param label            ノード名
 * @return    true=存在する
 */
bool UdmConfigBase::checkNode(const std::string node)
{
    if (!this->parser) {
        return false;
    }

    // ノードがあるかチェック
    return this->parser->chkNode(node);
}


/**
 * DFIファイルからラベルの要素リストを取得する.
 * カレントノードを移動して配下のLabel,Nodeを取得する.
 * @param label            ラベル名
 * @param values        取得ラベルリスト
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::getLabels(const std::string& label, std::vector<std::string>& values)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }
    int ierror;
    int i, count;
    std::string parent_node, last_node;
    std::vector<std::string> labels, nodes;
    std::vector<std::string> results;

    // 親ノードの移動
    count = this->split(label, '/', results);
    if (count == 1) {
        parent_node = "/" + results[0];
    }
    else if (count > 1) {
        for (i=0; i<count-1; i++) {
            parent_node += "/" + results[i];
        }
        last_node = results[count-1];
    }
    else {
        return UDM_ERROR_PARAMETER;
    }

    ierror = this->parser->changeNode(parent_node);
    if (ierror != TP_NO_ERROR) {
        return UDM_ERROR;
    }

    // Labelの取得
    ierror = this->parser->getLabels(labels);
    if (ierror != TP_NO_ERROR) {
        this->parser->changeNode("/");
        return UDM_ERROR;
    }
    // Nodeの取得
    ierror = this->parser->getNodes(nodes);
    if (ierror != TP_NO_ERROR) {
        this->parser->changeNode("/");
        return UDM_ERROR;
    }

    this->parser->changeNode("/");

    // 取得ラベルの作成
    std::vector<std::string>::iterator itr;
    for (itr = labels.begin(); itr != labels.end(); itr++) {
        if (last_node.empty()) {
            values.push_back(parent_node + "/" + *itr);
        }
        else {
            if (!strcasecmp(last_node.c_str(), (*itr).c_str())) {
                values.push_back(parent_node + "/" + *itr);
            }
        }
    }
    for (itr = nodes.begin(); itr != nodes.end(); itr++) {
        if (last_node.empty()) {
            values.push_back(parent_node + "/" + *itr);
        }
        else {
            if (!strcasecmp(last_node.c_str(), (*itr).c_str())) {
                values.push_back(parent_node + "/" + *itr);
            }
        }
    }
    return UDM_OK;
}

/**
 * bool文字列をパースする.
 * @param value        bool文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        true | false
 */
bool UdmConfigBase::convertBool(const std::string value, UdmError_t *ierror)
{
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), "true")) return true;
    if (!strcasecmp(value.c_str(), "false")) return false;
    if (!strcasecmp(value.c_str(), "1"))     return true;
    if (!strcasecmp(value.c_str(), "0"))     return false;
    if (!strcasecmp(value.c_str(), "on"))    return true;
    if (!strcasecmp(value.c_str(), "off"))   return false;
    if (!strcasecmp(value.c_str(), "yes"))   return true;
    if (!strcasecmp(value.c_str(), "no"))    return false;
    if (!strcasecmp(value.c_str(), "with"))  return true;
    if (!strcasecmp(value.c_str(), "without"))  return false;
    if (!strcasecmp(value.c_str(), "enable"))   return true;
    if (!strcasecmp(value.c_str(), "disable"))  return false;

    *ierror = UDM_ERROR;
    return false;
}

/**
 * DataType文字列をパースする.
 * @param value        DataType文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        データ型:UDM_DATATYPE_ENUM
 */
UdmDataType_t UdmConfigBase::convertDataType(const std::string value, UdmError_t *ierror)
{
    UdmDataType_t dtype_val = Udm_DataTypeUnknown;
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), "Integer")) return Udm_Integer;
    if (!strcasecmp(value.c_str(), "LongInteger")) return Udm_LongInteger;
    if (!strcasecmp(value.c_str(), "RealSingle"))  return Udm_RealSingle;
    if (!strcasecmp(value.c_str(), "RealDouble"))  return Udm_RealDouble;
    if (!strcasecmp(value.c_str(), "String"))      return Udm_String;
    if (!strcasecmp(value.c_str(), "Boolean"))     return Udm_Boolean;
    if (!strcasecmp(value.c_str(), "Numeric"))    return Udm_Numeric;

    *ierror = UDM_ERROR;
    return dtype_val;
}


/**
 * FileCompositionType文字列をパースする.
 * @param value        FileCompositionType文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        CGNSファイル構成タイプ : UdmFileCompositionType_t
 */
UdmFileCompositionType_t UdmConfigBase::convertFileCompositionType(const std::string value, UdmError_t *ierror)
{
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), "IncludeGrid")) return Udm_IncludeGrid;
    else if (!strcasecmp(value.c_str(), "ExcludeGrid")) return Udm_ExcludeGrid;
    else if (!strcasecmp(value.c_str(), "AppendStep"))  return Udm_AppendStep;
    else if (!strcasecmp(value.c_str(), "EachStep"))  return Udm_EachStep;
    else if (!strcasecmp(value.c_str(), "GridConstant"))      return Udm_GridConstant;
    else if (!strcasecmp(value.c_str(), "GridTimeSlice"))     return Udm_GridTimeSlice;
    else *ierror = UDM_ERROR;

    return Udm_FileCompositionTypeUnknown;
}


/**
 * Double文字列をパースする.
 * @param value        Double文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        Double数値
 */
double UdmConfigBase::convertDouble(const std::string& value, UdmError_t *ierror)
{
    *ierror = UDM_OK;

    TextParser parser;
    double value_real = parser.convertDouble(value, (int*)ierror);
    if (*ierror != (int)TP_NO_ERROR) {
        *ierror = UDM_ERROR;
    }
    return value_real;
}


/**
 * Long文字列をパースする.
 * @param value        Long文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        Long数値
 */
long long UdmConfigBase::convertLong(const std::string& value, UdmError_t *ierror)
{
    *ierror = UDM_OK;

    TextParser parser;
    long long value_long = parser.convertLongLong(value, (int*)ierror);
    if (*ierror != (int)TP_NO_ERROR) {
        *ierror = UDM_ERROR;
    }
    return value_long;
}


/**
 * UdmSize_t文字列をパースする.
 * @param value        UdmSize_t文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        Long数値
 */
UdmSize_t UdmConfigBase::convertSize_t(const std::string& value, UdmError_t *ierror)
{
    *ierror = UDM_OK;

    UdmSize_t value_size;
    std::istringstream istr(value);
    istr >> value_size;
    return value_size;
}


/**
 * GridLocation文字列をパースする.
 * @param value        GridLocation文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        CGNS:配置位置 : GridLocation
 */
UdmGridLocation_t UdmConfigBase::convertGridLocation(const std::string value, UdmError_t *ierror)
{
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), "Vertex")) return Udm_Vertex;
    if (!strcasecmp(value.c_str(), "CellCenter")) return Udm_CellCenter;

    *ierror = UDM_ERROR;
    return Udm_GridLocationUnknown;
}


/**
 * VectorType文字列をパースする.
 * @param value        VectorType文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        ベクトルタイプ : VectorType
 */
UdmVectorType_t UdmConfigBase::convertVectorType(const std::string value, UdmError_t *ierror)
{
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), "Scalar")) return Udm_Scalar;
    if (!strcasecmp(value.c_str(), "Vector")) return Udm_Vector;

    *ierror = UDM_ERROR;
    return Udm_VectorTypeUnknown;
}


/**
 * ラベルのDFIファイル内の出現回数を取得する.
 * 添字(="[X]")が付いているラベルはその出現数を取得する.
 * ラベルには添字を付けないこと。付けた場合は、添字も含めたラベルの一致をチェックする。
 * @param label            ラベル名
 * @return        ラベル出現数
 */
int UdmConfigBase::countLabel(const std::string label)
{
    int ierror;
    std::string node,str,chkstr="";
    std::vector<std::string> labels;
    int len=label.length();
    int inode=0;
    int next=0;

    if (!this->parser) {
        return 0;
    }

    // Nodeがあるかチェック
    ierror = this->getLabels(label, labels);
    if (ierror != 0){
        return 0;
    }

    for (int i = 0; i < labels.size(); i++) {
        if( !strcasecmp(labels[i].c_str(), label.c_str()) ){
            inode++;
            continue;
        }

        node = labels[i].substr(0,len);
        if( !strcasecmp(node.c_str(), label.c_str()) ){
            if (labels[i].length() <= len) continue;

            str = labels[i].substr(len+1);
            next = str.find("/");
            if(next == 0) {
                inode++;
            }
            else{
                if(chkstr!=str.substr(0,next)){
                    chkstr=str.substr(0,next);
                    inode++;
                }
            }
        }
    }
    return inode;
}


/**
 * DFIラベルから末尾の添字("[]")のインデックスを取得する.
 * @param label            DFIラベル
 * @return            添字("[]")のインデックス, 添字が数値ではない場合、取得エラーの場合、-1を返す。
 */
int UdmConfigBase::splitLabelIndex(const std::string& label) const
{
    unsigned int start, end;
    std::string id;
    int result = -1;

    start = label.find_last_of('[');
    if (start == std::string::npos) return -1;
    if (start > label.length()-2) return -1;
    end = label.find_last_of(']');
    if (end == std::string::npos) return -1;
    if (start >= end) return -1;
    if (end-start-1 > label.length()) return -1;
    id = label.substr(start+1, end-start-1);

    // 文字列の数値チェック
    if (!udm_is_digits(id.c_str())) {
        return -1;
    }
    result = atoi(id.c_str());
    if (result == 0 && id != std::string("0")) {
        return -1;
    }

    return result;
}



/**
 * DFIファイルへラベルの値を設定する:文字列.
 * @param  label        ラベル名
 * @param  value        値
 * @param  data_type    データ型
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::setValue(const std::string& label, const std::string& value, UdmDataType_t  data_type)
{
    if (!this->parser) {
        return UDM_ERROR_TEXTPARSER_NULL;
    }
    TextParserError error;
    if (this->checkLabel(label)) {
        // 既存要素はupdateValueを行う.
        error = this->parser->updateValue(label, value);
    }
    else {
        std::string  create_value;
        // 新規要素はcreateLeafを行う.
        if (this->isListValue(value)) {
            create_value = value;
        }
        else if (data_type == Udm_String) {
            create_value = "\"" + value + "\"";
        }
        else {
            create_value = value;
        }
        error = this->parser->createLeaf(label, create_value);
    }
    if (error != TP_NO_ERROR) {
        return UDM_ERROR;
    }
    return UDM_OK;
}


/**
 * DFIファイルへラベルの値を設定する:整数.
 * @param  label        ラベル名
 * @param  value        値
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::setValue(const std::string& label, long long value)
{
    std::string buf;
    this->toString(value, buf);
    return this->setValue(label, buf, Udm_LongInteger);
}

/**
 * DFIファイルへラベルの値を設定する:整数.
 * @param  label        ラベル名
 * @param  value        値
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::setValue(const std::string& label, UdmSize_t value)
{
    std::string buf;
    this->toString(value, buf);
    return this->setValue(label, buf, Udm_LongInteger);
}

/**
 * DFIファイルへラベルの値を設定する:実数.
 * @param  label        ラベル名
 * @param  value        値
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::setValue(const std::string& label, double value)
{
    std::string buf;
    this->toString(value, buf);
    return this->setValue(label, buf, Udm_RealDouble);

}

/**
 * DFIファイルへラベルの値リストを設定する:文字列.
 * @param  label        ラベル名
 * @param  value        値リスト:文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::setValueList(const std::string& label, std::vector<std::string> values)
{
    std::string buf;
    std::vector<std::string>::iterator itr;
    for(itr = values.begin(); itr != values.end(); ++itr) {
        if (!buf.empty()) buf += ", ";
        buf += "\"" + *itr + "\"";
    }
    if (buf.empty()) return UDM_ERROR;
    buf = "(" + buf + ")";

    return this->setValue(label, buf);

}


/**
 * DfiType文字列をパースする.
 * @param value        DfiType文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        DFI種別 : UdmDfiType_t
 */
UdmDfiType_t UdmConfigBase::convertDfiType(std::string value, UdmError_t *ierror)
{
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), UDM_DFI_DFITYPE_UNS)) return Udm_UnsDfiType;
    *ierror = UDM_ERROR;
    return Udm_DfiTypeUnknown;
}

/**
 * ファイル命名書式:FieldFilenameFormat文字列をパースする.
 * @param value        FieldFilenameFormat文字列
 * @param ierror    エラー番号 (UDM_OK | UDM_ERROR)
 * @return        ファイル命名書式:UdmFieldFilenameFormat_t
 */
UdmFieldFilenameFormat_t UdmConfigBase::convertFieldFilenameFormat(std::string value, UdmError_t *ierror)
{
    *ierror = UDM_OK;
    if (!strcasecmp(value.c_str(), "step_rank")) return Udm_step_rank;
    if (!strcasecmp(value.c_str(), "rank_step")) return Udm_rank_step;
    *ierror = UDM_ERROR;
    return Udm_FieldFilenameFormatUnknown;
}


/**
 * DFI設定値がリスト表記であるかチェックする.
 * @param value        DFI設定値
 * @return        true=リスト表記
 */
bool UdmConfigBase::isListValue(const std::string &value)
{
    std::string start, end, tolower;

    if (value.empty()) return false;

    // (...)文字列リスト表記
    start = value.substr(0, 1);
    end = value.substr(value.length()-1, 1);
    if (start == "(" && end == ")") {
        return true;
    }

    // @list : 数値リスト表記
    start = value.substr(0, 5);
    end = value.substr(value.length()-1, 1);
    // 小文字に変換
    std::transform(start.begin(), start.end(), start.begin(), ::tolower);
    if (start == "@list" && end == ")") {
        return true;
    }

    // @range : 数値範囲表記
    start = value.substr(0, 6);
    end = value.substr(value.length()-1, 1);
    // 小文字に変換
    std::transform(start.begin(), start.end(), start.begin(), ::tolower);
    if (start == "@range" && end == ")") {
        return true;
    }

    return false;
}

/**
 * 出力ファイルにラベルを出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeLabel(FILE *fp, unsigned int indent, const std::string &label)
{
    unsigned int i=0;

    if (fp == NULL) return UDM_ERROR;
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s {\n", label.c_str());

    return UDM_OK;
}


/**
 * 出力ファイルに"ラベル=設定値"を出力する:文字列.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @param value        出力設定値:文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, std::string &value)
{
    unsigned int i=0;

    if (fp == NULL) return UDM_ERROR;
    if (label.empty()) return UDM_ERROR;
    if (value.empty()) return UDM_ERROR;
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s = \"%s\"\n", label.c_str(), value.c_str());

    return UDM_OK;
}

/**
 * 出力ファイルに"ラベル=設定値"を出力する:int.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @param value        出力設定値:long long
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, int value)
{
    return this->writeNodeValue(fp, indent, label, (long long)value);
}

/**
 * 出力ファイルに"ラベル=設定値"を出力する:long long.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @param value        出力設定値:long long
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, long long value)
{
    unsigned int i=0;
    std::string buf;
    if (fp == NULL) return UDM_ERROR;
    if (this->toString(value, buf) != UDM_OK) {
        return UDM_ERROR;
    }
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s = %s\n", label.c_str(), buf.c_str());

    return UDM_OK;
}


/**
 * 出力ファイルに"ラベル=設定値"を出力する:UdmSize_t.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @param value        出力設定値:UdmSize_t
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, UdmSize_t value)
{
    unsigned int i=0;
    std::string buf;
    if (fp == NULL) return UDM_ERROR;
    if (this->toString(value, buf) != UDM_OK) {
        return UDM_ERROR;
    }
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s = %s\n", label.c_str(), buf.c_str());

    return UDM_OK;
}

/**
 * 出力ファイルに"ラベル=設定値"を出力する:double.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @param value        出力設定値:double
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, double value)
{
    unsigned int i=0;
    std::string buf;
    if (fp == NULL) return UDM_ERROR;
    if (this->toString(value, buf) != UDM_OK) {
        return UDM_ERROR;
    }
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s = %s\n", label.c_str(), buf.c_str());

    return UDM_OK;
}

/**
 * 出力ファイルに"ラベル=設定値"を出力する:文字列リスト.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param label        出力ラベル
 * @param value        出力設定値:文字列リスト
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeNodeValue(FILE *fp, unsigned int indent, const std::string &label, std::vector<std::string> values)
{
    unsigned int i=0;
    std::string buf;
    if (fp == NULL) return UDM_ERROR;

    std::vector<std::string>::iterator itr;
    for(itr = values.begin(); itr != values.end(); ++itr) {
        if (!buf.empty()) buf += ", ";
        buf += "\"" + *itr + "\"";
    }
    if (buf.empty()) return UDM_ERROR;
    buf = "(" + buf + ")";
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s = %s\n", label.c_str(), buf.c_str());

    return UDM_OK;

}


/**
 * 出力ファイルに文字列を出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @param text        出力文字列
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeText(FILE *fp, unsigned int indent, const std::string &text)
{
    unsigned int i=0;

    if (fp == NULL) return UDM_ERROR;
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "%s\n", text.c_str());

    return UDM_OK;
}


/**
 * 出力ファイルに閉じタブ('}')を出力する.
 * @param fp        出力ファイルポインタ
 * @param indent    インデント
 * @return        エラー番号 (UDM_OK | UDM_ERROR)
 */
UdmError_t UdmConfigBase::writeCloseTab(FILE *fp, unsigned int indent)
{
    unsigned int i=0;

    if (fp == NULL) return UDM_ERROR;
    for (i=0; i<indent; i++) fprintf(fp, UDM_DFI_INDENT);
    fprintf(fp, "}\n");

    return UDM_OK;
}


} /* namespace udm */
