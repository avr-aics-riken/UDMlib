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
 * @file UdmStaging.h
 * @brief Staging Class Header
 */

#ifndef _UDMSTAGING_H_
#define _UDMSTAGING_H_

// Moved here for the Intel Compiler
#include "model/UdmModel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define UDMFRM_OUTPUT_RANKFORMAT            "%06d"
#define UDMFRM_NUM_TIMESLICE_DIRECTORY      10

namespace udm {
/**
 * ステージングクラス
 */
class UdmStaging {
private:
    std::string input_dir;              ///< DFIディレクトリ
    std::string output_dir;             ///< 出力ディレクトリ
    std::string index_dfi;            ///< index.dfiファイル名
    std::string udmlib_file;            ///< udplib.tpファイル名
    int stepno;                         ///< ステップ番号
    int num_procs;                       ///< 振分プロセス数 ：引数のnpの数
    UdmDfiConfig  dfi_config;           ///< UdmDfiConfig
    bool debug_trace;                    ///< デバッグトレース（コピーファイル表示）
    bool print_version;                    ///< バージョン表示

public:

    /** コンストラクタ **/
    UdmStaging();
    /** デストラクタ **/
    ~UdmStaging();

    bool readDfi();
    bool readDfi(const char* index_dfi);
    bool mappingFiles();
    bool mappingFiles(int num_procs);
    const UdmDfiConfig& getDfiConfig() const;
    const std::string& getIndexDfi() const;
    void setIndexDfi(const std::string& filename);
    const std::string& getInputDirectory() const;
    void setInputDirectory(const std::string& path);
    int getNumProcess() const;
    void setNumProcess(int num);
    const std::string& getOutputDirectory() const;
    void setOutputDirectory(const std::string& path);
    bool isDebugTrace() const;
    void setDebugTrace(bool debugTrace);
    bool isPrintVersion() const;
    void setPrintVersion(bool printVersion);
    void print_info() const;
    int getStepno() const;
    void setStepno(int stepno);
    const std::string& getUdmlibFile() const;
    void setUdmlibFile(const std::string& udmlibFile);

private:
    void initialize();
    bool copyCgnsFiles(const char *input_dir,
                       const char *output_dir,
                       const char *prefix,
                       int rankno,
                       const char *field_directory,
                       bool timeslicedirectory) const;
    bool writeCgnsFiles(const char *index_dfi,
                        const char *read_cgns,
                        const char *output_dir,
                        int rankno,
                        int stepno) const;
    bool writeDfiFiles( const char *index_dfi,
                        const char* output_dir,
                        int rankno,
                        int stepno) const;
    int getFileList(const std::string &input_dir, std::vector<std::string> &filelist) const;
    int getDirectoryList(const std::string& dirname, std::vector<std::string>& dirlist) const;
    bool existsFile(const char *filename) const;
    bool existsDirectory(const char *dirname) const;
    bool checkCgnsFile(const char *filename, const char *prefix, int rankno) const;
    bool checkTimeSliceDirectory(const char *directory) const;
    bool copyFile(const char *src_path,
                  const char *dest_path) const;

};

} /* namespace udm */

#endif //_UDMSTAGING_H_
