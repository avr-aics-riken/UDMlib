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

#ifndef _UDMFILEINFOCONFIG_H_
#define _UDMFILEINFOCONFIG_H_

/**
 * @file UdmFileInfoConfig.h
 * index.dfiファイルのFileInfoデータクラスのヘッダーファイル
 */
#include "config/UdmConfigBase.h"
#include "config/UdmDfiValue.h"

namespace udm {

/**
 * index.dfiファイルのFileInfoデータクラス
 */
class UdmFileInfoConfig: public UdmConfigBase {
private:
    UdmDfiType_t     dfi_type;                ///< DFI種別
    std::string      directory_path;            ///< フィールドデータディレクトリ
    bool            timeslice_directory;    ///< 時刻ディレクトリ作成オプション
    std::string      prefix;                    ///< ベースファイル名
    std::string      fileformat;                ///< ファイルフォーマット
    UdmFieldFilenameFormat_t     fieldfilename_format;        ///< ファイル命名書式
    std::string     fieldfilename;        ///< ファイル名
    std::string       element_path;            ///< CGNSパス名
    std::vector<UdmFileCompositionType_t>     filecomposition_types;    ///< CGNSファイル構成タイプ

public:
    UdmFileInfoConfig();
    UdmFileInfoConfig(TextParser *parser);
    virtual ~UdmFileInfoConfig();
    UdmError_t read();
    UdmError_t write(FILE *fp, unsigned int indent);

    UdmDfiType_t getDfiType() const;
    void setDfiType(UdmDfiType_t dfiType);
    UdmError_t getDirectoryPath(std::string& directorypath) const;
    void setDirectoryPath(const std::string& directoryPath);
    UdmError_t getElementPath(std::string& elementpath) const;
    void setElementPath(const std::string& elementPath);
    UdmFieldFilenameFormat_t getFieldfilenameFormat() const;
    void setFieldfilenameFormat(UdmFieldFilenameFormat_t fieldfilenameFormat);
    int getNumFileCompositionTypes() const;
    const std::vector<UdmFileCompositionType_t>& getFileCompositionTypes() const;
    void setFileCompositionTypes(const std::vector<UdmFileCompositionType_t>& types);
    void setFileCompositionType(UdmFileCompositionType_t type);
    void removeFileCompositionType(UdmFileCompositionType_t type);
    bool existsFileCompositionType(UdmFileCompositionType_t type) const;
    UdmError_t getFileformat(std::string& fileformat) const;
    void setFileformat(const std::string& fileformat);
    UdmError_t getPrefix(std::string&  prefix) const;
    void setPrefix(const std::string& prefix);
    bool isTimeSliceDirectory() const;
    void setTimeSliceDirectory(bool timesliceDirectory);
    UdmError_t getDfiValue(const std::string &label_path, std::string &value) const;
    UdmError_t setDfiValue(const std::string &label_path, const std::string &value);
    int getNumDfiValue(const std::string &label_path) const;
    UdmError_t getCgnsLinkFile(std::string &file_name, int rank_no) const;
    const std::string& getFieldFilename() const;
    void setFieldFilename(const std::string& filename);
    bool isFileGridConstant() const;
    bool isFileGridCoordinates() const;
    bool isFileFlowSolution() const;
    bool isFileTimeSlice() const;
    bool isFileIncludeGrid() const;
    UdmError_t generateCgnsTimeSliceName(std::string &file_name, int rank_no, int timeslice_step) const;
    UdmError_t generateCgnsFileName(std::string &file_name, int rank_no) const;
    UdmError_t generateCgnsLinkName(std::string &file_name, int rank_no) const;
    UdmError_t generateCgnsTimeSliceGridName(std::string &file_name, int rank_no, int timeslice_step) const;
    UdmError_t generateCgnsTimeSliceSolutionName(std::string &file_name, int rank_no, int timeslice_step) const;
    UdmError_t generateCgnsGridName(std::string &file_name, int rank_no) const;
    UdmError_t generateCgnsSolutionName(std::string &file_name, int rank_no) const;

protected:

private:
    void initialize();    UdmError_t createTimeSliceDirectoryPath(std::string &file_name, int timeslice_step) const;

};

} /* namespace udm */

#endif /* _UDMFILEINFOCONFIG_H_ */
