/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */


/**
 * @file udm_pathutils.h
 * ファイル、フォルダ操作関数群のファイル
 */

#ifndef _UDM_PATHUTILS_H_
#define _UDM_PATHUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#endif
// for sendfile
#if defined(__APPLE__)
#include <sys/socket.h>
#include <sys/uio.h>
#else
#include <sys/sendfile.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define UDM_PATH_MAX  4096

#ifdef __cplusplus
namespace udm {
#endif

inline char udm_get_pathdelimiter()
{
#if defined(_WIN32) || defined(WIN32)     /* win32 */
  return '\\';
#else        /* Limux */
  return '/';
#endif
}

/**
 * ファイルパス文字列を分解する.
 * @param [in]  path        分解元ファイルパス文字列
 * @param [out] drive        ドライブ名（Linuxの場合はNULLを返す）
 * @param [out] folder        フォルダ名
 * @param [out] name        ファイル名
 * @param [out] ext            拡張子
 */
inline void udm_splitpath(const char* path, char* drive, char* folder, char* name, char* ext)
{
    if (path == NULL) return;

    const char *end, *p, *s;
    char x;

    // ドライブ名の取得
    x = path[0];
    if ( ((x >= 'A' && x <= 'Z' ) || (x >= 'a' && x <= 'z')) && path[1] == ':' ) {
        if (drive) {
            *drive++ = *path++;
            *drive++ = *path++;
            *drive = '\0';
        }
    }
    else if (drive) {
        *drive = '\0';
    }

    for (end=path; *end && *end!=':'; ) {
        end++;
    }

    // 拡張子'.'の検索
    for (p=end; p>path && *--p!='\\' && *p!='/'; ) {
        if (*p == '.') {
            end = p;
            break;
        }
    }

    if (ext && *end != '\0') {
        for(s=end; (*ext=*s++); ) ext++;
    }

    // 末尾からパス区切りの検索
    for (p=end; p>path; ) {
        if (*--p=='\\' || *p=='/') {
            p++;
            break;
        }
    }
    // ファイル名の取得
    if (name) {
        for (s=p; s<end; ) *name++ = *s++;
        *name = '\0';
    }

    // フォルダ名の取得
    if (folder) {
        for (s=path; s<p; ) *folder++ = *s++;
        *folder = '\0';
    }

    return;
}

/**
 * パス名がドライブ名を含んだ表記であるかチェックする.
 * @param path        パス名
 * @return        true=ドライブ名を含む. (Linuxの場合はfalseを返す)
 */
inline bool udm_has_drivepath(const char* path)
{
    if (path == NULL) return false;

#if defined(_WIN32) || defined(WIN32)     /* win32 */
    if ( strlen(path) < 2 ) return false;
    char x = path[0];
    if ( ((x >= 'A' && x <= 'Z' ) || (x >= 'a' && x <= 'z')) && path[1] == ':' ) {
        return true;
    }
    return false;
#else     /* linux */
    return false;
#endif
}

/**
 * パス名からドライブ名を除いた文字列を取得する.
 * returns drive (ex. 'C:').
 * @param [in]  path        パス名
 * @param [out] emit_drive        ドライブ名を除いたパス名 (Linuxの場合は、pathと同じ)
 */
inline void udm_emit_drivepath(const char* path, char *emit_drive)
{
    if (path == NULL) return;
    if (emit_drive == NULL) return;

    if ( ! udm_has_drivepath(path) ) {
        strcpy(emit_drive, path);
        return;
    }
    strcpy(emit_drive, path+2);

    return;
}

/**
 * パス名が絶対パスであるかチェックする.
 * @param path        パス名
 * @return        true  : Absolute Path(絶対パス), false : Relative Path(相対パス)
 */
inline bool udm_is_absolutepath(const char* path)
{
    if (path == NULL) return false;

    char xpath[UDM_PATH_MAX] = {0x00};
    udm_emit_drivepath(path, xpath);
    char c1 = xpath[0];
    return (c1 == '\\' || c1 == '/');
}

/**
 * パス名からフォルダ名のみ取得する.
 * パス名の末尾からパス区切りを検索するのみであるので、フォルダパスの場合は最後のフォルダ名は削除される.
 * @param path        パス名
 */
inline void udm_get_dirname(const char* path, char* dirname)
{
    if (path == NULL) return;
    if (dirname == NULL) return;

    char drive[UDM_PATH_MAX] = {0x00};
    char folder[UDM_PATH_MAX] = {0x00};
    int len = 0;

    udm_splitpath(path, drive, folder, NULL, NULL);
    if (strlen(drive) == 0) {
        strcpy(dirname, folder);
    }
    else {
        sprintf(dirname, "%s%s", drive, folder);
    }
    len = strlen(dirname);
    if (len <= 0) return;
    if (dirname[len-1] == '\\' || dirname[len-1] == '/') {
        dirname[len-1] = '\0';
    }

    return;
}

/**
 * パス名からファイル名のみ取得する.
 * @param path        パス名
 */
inline void udm_get_filename(const char *path, char *filename)
{
    if (path == NULL) return;
    if (filename == NULL) return;

    char drive[UDM_PATH_MAX] = {0x00};
    char folder[UDM_PATH_MAX] = {0x00};
    char name[UDM_PATH_MAX] = {0x00};
    char ext[UDM_PATH_MAX] = {0x00};
    udm_splitpath(path, drive, folder, name, ext);
    if (strlen(ext) > 0) {
        sprintf(filename, "%s%s", name, ext);
    }
    else {
        sprintf(filename, "%s", name);
    }

    return;
}

/**
 * フォルダ名とファイル名を結合してパス名を取得する.
 * @param [in]  dirname        フォルダ名
 * @param [in]  fname            ファイル名
 * @param [out] path            パス名
 */
inline void udm_connect_path( const char *dirname, const char *fname, char *path)
{
    if (dirname == NULL) return;
    if (fname == NULL) return;
    if (path == NULL) return;

    if (strlen(dirname) == 0) {
        sprintf(path, "%s", fname);
    }
    else if (strlen(fname) == 0) {
        sprintf(path, "%s", dirname);
    }
    else {
        char end = dirname[strlen(dirname)-1];
        if ( (end == '\\' || end == '/')) {
            sprintf(path, "%s%s", dirname, fname);
        }
        else if ( strchr( dirname, '\\' ) != NULL) {
            sprintf(path, "%s\\%s", dirname, fname);
        }
        else if ( strchr( dirname, '/' ) != NULL) {
            sprintf(path, "%s/%s", dirname, fname);
        }
        else {
            sprintf(path, "%s%c%s", dirname, udm_get_pathdelimiter(), fname);
        }
    }

    return;
}


/**
 * パス名から拡張子を削除する.
 * @param path        パス名
 * @param extract    拡張子削除ファイル名
 */
inline void udm_extract_pathext(const char *path, char* extract)
{
    if (path == NULL) return;
    if (extract == NULL) return;

    const char *p, *end, *ext;

    // 拡張子'.'の検索
    ext = NULL;
    end = path + strlen(path);
    for (p=end; p>path && *--p!='\\' && *p!='/'; ) {
        if (*p == '.') {
            ext = p;
            break;
        }
    }
    if (ext == NULL) {
        ext = end;
    }
    if (extract) {
        for (p=path; path<ext; ) {
            *extract++ = *path++;
        }
        *extract = '\0';
    }

    return;
}

/**
 * パス名から再帰的にディレクトリの作成を行う
 * @param path        パス名
 * @return    エラー番号 : 成功した場合 0 を、失敗した場合 -1 を返す
 */
inline int udm_make_directories(const char *path)
{
    if (path == NULL) return -1;
    if (strlen(path) <= 0) return -1;

    umask(022);
    int ret = 0;
#if defined(_WIN32)
    ret = _mkdir(path);
#else
    ret = mkdir(path, 0777);
#endif
    if( ret != 0 ) {
        if( errno == EEXIST ) return 0;

        // 親ディレクトリを取得する.
        char parent[256] = {0x00};
        udm_get_dirname(path, parent);
        if (strcmp(path, parent) == 0) return ret;

        // 親ディレクトリを作成する.
        int ret2 = udm_make_directories( parent );
        if( ret2 != 0 ) {
            return ret2;
        }
        // 子ディレクトリを作成する.
        ret = udm_make_directories( path );
    }

    return ret;
}

/**
 * パスから指定階層のディレクトリ名を取得する.
 * 先頭が'/',ドライブ名の場合は、level=0は'/',ドライブ名とする.
 * paht = /path1/path2/path3
 * level = 3
 * level_path = path2
 * return = 4
 * @param path        パス
 * @param level        取得階層
 * @param level_path    階層ディレクトリ名
 * @return            全ディレクトリ階層数
 */
inline int udm_get_directory_level(const char *path, int level, char *level_path)
{
    if (level_path == NULL) return 0;
    if (path == NULL) return 0;
    int path_len = strlen(path);
    if (path_len <= 0) return 0;

    char drive[64] = {0x00};
    char part_path[256] = {0x00};
    int i = 0;
    int path_level = 0;

    // ドライブ名の取得
    i = 0;

#if defined(_WIN32) || defined(WIN32)     /* win32 */
    char x;
    x = path[0];
    if ( ((x >= 'A' && x <= 'Z' ) || (x >= 'a' && x <= 'z')) && path[1] == ':' ) {
        drive[i++] = *path++;
        drive[i++] = *path++;
    }
#endif

    do {
        if (*path=='\\'||*path=='/') {
            drive[i++] = *path;
            path++;
        }
        else {
            break;
        }
    } while ( *path!='\\'&&*path!='/' );
    drive[i] = '\0';
    if (i>0) {
        if (path_level == level) {
            strcpy(level_path, drive);
        }
        path_level++;
    }

    i = 0;
    do {
        if (*path=='\\'||*path=='/') {
            if (path_level == level) {
                strcpy(level_path, part_path);
            }
            if (i>0) path_level++;
            i = 0;
            part_path[0] = '\0';
        }
        else {
            part_path[i++] = *path;
            part_path[i] = '\0';
        }
        path++;
    } while ( *path!='\0' );

    if (strlen(part_path) > 0) {
        if (path_level == level) {
            strcpy(level_path, part_path);
        }
        path_level++;
    }

    return path_level;
}

/**
 * FROMパスから見たTOパスの相対パスを取得する.
 * FROMパス, TOパスが相対、絶対パスであり、相対パスを取得できない場合は、TOパスを返す.
 * (1) FROMパス:frompath/test_path
 *     TOパス:frompath/test_path/to_path
 *     相対パス:to_path
 * (2) FROMパス:frompath/test_path/samples
 *     TOパス:to_path
 *     相対パス:../../../to_path
 * @param [in]  from_path            FROMパス
 * @param [in]  to_path            TOパス
 * @param [out] relative_path        相対パス
 */
inline void udm_get_relativepath(const char *from_path, const char *to_path, char *relative_path)
{
    if (relative_path == NULL) return;
    if (to_path == NULL) return;
    if (from_path == NULL) {
        strcpy(relative_path, to_path);
        return;
    }

    // 同じ相対、絶対パスでない場合は、相対パスは取得できない.
    if (udm_is_absolutepath(from_path) ^ udm_is_absolutepath(to_path)) {
        strcpy(relative_path, to_path);
        return;
    }

    // 先頭パスが同じであるか?
    int level = 0;
    int num_from_path = 0;
    char from_level_path[256] = {0x00};
    int num_to_path = 0;
    char to_level_path[256] = {0x00};
    int start_pos = 0;
    int i;
    num_from_path = udm_get_directory_level(from_path, level, from_level_path);
    num_to_path = udm_get_directory_level(to_path, level, to_level_path);
    if (strcmp(from_level_path, to_level_path) == 0) {
        // 同一パスを削除する.
        for (level=1; level<num_from_path || level<num_to_path; level++) {
            from_level_path[0] = '\0';
            to_level_path[0] = '\0';
            num_from_path = udm_get_directory_level(from_path, level, from_level_path);
            num_to_path = udm_get_directory_level(to_path, level, to_level_path);
            if (strlen(from_level_path) == 0) break;
            if (strlen(to_level_path) == 0) break;
            if (strcmp(from_level_path, to_level_path) != 0) break;

            start_pos = level;
        }
    }
    else {
        // 絶対パスで先頭がことなることはない。Windowsでドライブ番号が異なる場合のみ
        if (udm_is_absolutepath(from_path)) {
            strcpy(relative_path, to_path);
            return;
        }
    }

    // FROMパス分"../"を組み立てる。
    relative_path[0] = '\0';
    for (i=start_pos+1; i<num_from_path; i++) {
        strcat(relative_path, "../");
    }

    // TOパスの残りを組み立てる
    for (level=start_pos+1; level<num_to_path; level++) {
        udm_get_directory_level(to_path, level, to_level_path);
        if (level!=start_pos+1) strcat(relative_path, "/");
        strcat(relative_path, to_level_path);
    }

    return;
}


/**
 * ファイルをコピーする
 * @param from_path            コピー元ファイルパス
 * @param to_path            コピー先ファイルパス
 * @return     0=success, 0以外=failure
 */
inline int udm_copyfile(const char *from_path, const char *to_path)
{
    int read_fd;
    int write_fd;
    struct stat stat_buf;
    off_t offset = 0;

    // コピー元ファイルオープン
    read_fd = open (from_path, O_RDONLY);
    if (read_fd == -1) {
        return -1;
    }

    // コピー元ファイルサイズの取得
    fstat (read_fd, &stat_buf);

    // コピー先ファイルオープン
    write_fd = open (to_path, O_WRONLY | O_CREAT, stat_buf.st_mode);
    if (write_fd == -1) {
        return -1;
    }

    // コピー元ファイルデータのコピー
#if defined(__APPLE__)
    off_t sbytes = stat_buf.st_size;
    sendfile (read_fd, write_fd, offset, &sbytes, NULL, 0);
#else
    sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
#endif

    // クローズ
    close (read_fd);
    close (write_fd);

    return 0;
}

/**
 * カレントディレクトリを取得する
 * @param [out] directory        取得カレントディレクトリ
 * @return            取得カレントディレクトリポインタ
 */
inline  char* udm_getcwd(char *directory)
{
    if (directory == NULL) return NULL;

    char cwd[UDM_PATH_MAX] = {0x00};
#if defined(_WIN32)
    _getcwd(cwd, UDM_PATH_MAX);
#else
    getcwd(cwd, UDM_PATH_MAX);
#endif
    strcpy(directory, cwd);
    return directory;
}

#ifdef __cplusplus
} /* namespace udm */
#endif

#endif /* _UDM_PATHUTILS_H_ */
