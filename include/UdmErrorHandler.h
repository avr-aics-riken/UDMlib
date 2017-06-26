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

#ifndef _UDMERRORHANDLER_H_
#define _UDMERRORHANDLER_H_

/**
 * @file UdmErrorHandler.h
 * エラーハンドラクラスのヘッダーファイル
 */

#include "UdmBase.h"
#include "udm_errorno.h"
#include <stdarg.h>

#define     UDM_DEFAULT_DEBUG_LEVEL            0x0002
#define     UDM_DEBUG_LEVEL_ERROR_MASK         0x00FF
#define     UDM_ERROR_BUFFER_SIZE              65536

// エラーハンドラマクロ
#define     UDM_ERRORNO_HANDLER(error_no)        UdmErrorHandler::getInstance()->error(error_no, __FILE__, __LINE__, __FUNCTION__)
#define     UDM_ERROR_HANDLER(error_no, fmt, ...)        UdmErrorHandler::getInstance()->error(error_no, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define     UDM_WARNINGNO_HANDLER(error_no)        UdmErrorHandler::getInstance()->warning(error_no, __FILE__, __LINE__, __FUNCTION__)
#define     UDM_WARNING_HANDLER(warning_no, fmt, ...)        UdmErrorHandler::getInstance()->warning(warning_no, __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define     UDM_INFO_HANDLER(fmt, ...)        UdmErrorHandler::getInstance()->info( __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define     UDM_DEBUG_HANDLER(fmt, ...)        UdmErrorHandler::getInstance()->debug( __FILE__, __LINE__, __FUNCTION__, fmt, ## __VA_ARGS__)
#define     UDM_INFO_PRINTF(fmt, ...)        UdmErrorHandler::getInstance()->info(fmt, ## __VA_ARGS__)
#define     UDM_DEBUG_PRINTF(fmt, ...)        UdmErrorHandler::getInstance()->debug(fmt, ## __VA_ARGS__)
#define     UDM_IS_DEBUG_LEVEL()        UdmErrorHandler::getInstance()->isDebugLevelPrintDebug()

namespace udm
{

/**
 * ノード、要素の仮想ノードタイプ
 */
typedef enum {
    Udm_ErrorHandlerUnknown,     //!< 未定
    Udm_ErrorLevel_Error,                //!< エラー  :  正常な動作を継続できないレベル
    Udm_ErrorLevel_Warning,                  //!< 警告  : 正常な動作は継続するがアプリに通知すべきレベル
    Udm_ErrorLevel_Info,                  //!< 情報   : 動作説明の情報出力
    Udm_ErrorLevel_Debug                  //!< デバッグレベルの出力
} UdmErrorLevel_t;

/**
 * エラーハンドラクラス
 */
class UdmErrorHandler
{
private:
    static UdmErrorHandler* m_instance;            ///< エラーハンドラインスタンス
    UdmError_t error_no;                ///< エラー番号
    std::string message;        ///< エラーメッセージ
    UdmErrorLevel_t level;
    std::string  error_filename;        ///< エラーファイル名
    int error_lineno;                    ///< エラーライン番号
    std::string  error_funcname;        ///< エラー関数

    /**
     * デバッグレベル : デフォルト 0x0002
     * 0x0000                エラーメッセージ出力なし：エラーメッセージはUdmErrorHandler::messageにセットするのみ。
     * 0x0001                エラーメッセージを常時出力する.
     * 0x0002                エラー、警告メッセージを常時出力する.
     * 0x0003                エラー、警告、情報メッセージを常時出力する.
     * 0x0004                エラー、警告、情報、デバッグメッセージを常時出力する.
     */
    unsigned int debug_level;

    /**
     * 出力先 : デフォルト 0x01
     * 0x00                    標準出力、ファイル出力なし
     * 0x01                    標準出力のみに出力
     * 0x02                    ファイルのみに出力
     * 0x03                    標準出力、ファイルに出力
     */
    unsigned int output_target;

private:
    UdmErrorHandler();
    virtual ~UdmErrorHandler();
    void setMessage(UdmErrorLevel_t level, UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* message);
    bool isDebugLevelSilent() const;
    bool isDebugLevelPrintError() const;
    bool isDebugLevelPrintWarning() const;
    bool isDebugLevelPrintInfo() const;
    void getMessage(char* buf, UdmErrorLevel_t level, UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* message);
    void printMessage(UdmErrorLevel_t level, UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* message);
    bool isOutputTargetStdout() const;
    bool isOutputTargetFile() const;

public:
    static UdmErrorHandler* getInstance();
    UdmError_t error(UdmError_t error_no, const char* filename = NULL, int lineno = 0, const char* funcname = NULL);
    UdmError_t error(UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* format, ...);
    UdmError_t error_info(UdmError_t error_no, const char* info);
    UdmError_t warning(UdmError_t error_no, const char* filename = NULL, int lineno = 0, const char* funcname = NULL);
    UdmError_t warning(UdmError_t error_no, const char* filename, int lineno, const char* funcname, const char* format, ...);
    UdmError_t warning_info(UdmError_t error_no, const char* info);
    void info(const char* filename, int lineno, const char* funcname, const char* format, ...);
    void info(const char* format, ...);
    void debug(const char* filename, int lineno, const char* funcname, const char* format, ...);
    void debug(const char* format, ...);
    void getMessage(char* message);
    void clear();
    void printMessage();
    unsigned int getDebugLevel() const;
    void setDebugLevel(unsigned int debug_level);
    unsigned int getOutputTarget() const;
    void setOutputTarget(unsigned int outputTarget);
    void writeFile(const char *msg);
    bool isDebugLevelPrintDebug() const;
};

} /* namespace udm */

#endif /* _UDMERRORHANDLER_H_ */
