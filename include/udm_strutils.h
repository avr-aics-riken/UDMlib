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
 * @file udm_strutils.h
 * 文字列操作関数群のファイル
 */

#ifndef _UDM_STRUTILS_H_
#define _UDM_STRUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Included for Intel Compiler
#include <ctype.h>

#ifdef __cplusplus
namespace udm {
#endif


/**
 * 文字列がすべて数値であるかチェックする.
 * @param [out]  str   チェック文字列
 * @return    true=すべて数値である
 */
inline bool udm_is_digits(const char *str)
{
    int len = strlen(str);
    int i = 0;
    for (i=0; i<len; i++) {
        if ( isdigit(str[i]) == 0 ) return false;
    }
    return true;
}

/**
 * 検索対象文字列から検索文字列の出現位置を大文字・小文字を比較しないで取得する.
 * 検索文字列が存在しない場合はNULLを返す。
 * @param s            検索対象文字列
 * @param find            検索文字列
 * @return            出現位置 : 検索文字列が存在しない場合はNULL
 */
inline char *udm_strcasestr(const char *s, const char *find)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != 0) {
        c = tolower((unsigned char)c);
        len = strlen(find);
        do {
            do {
                if ((sc = *s++) == 0)
                    return (NULL);
            } while ((char)tolower((unsigned char)sc) != c);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}


#ifdef __cplusplus
} /* namespace udm */
#endif

#endif /* _UDM_STRUTILS_H_ */
