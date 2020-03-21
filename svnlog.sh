#!/bin/bash

# 工作副本根目录: /home/leon/cdcode/unify_code/unify
# URL: svn://10.33.68.244/ROUTER/MESH/branch/unify
# Relative URL: ^/branch/unify
# 版本库根: svn://10.33.68.244/ROUTER/MESH


# 获取svn的根路径 svn://10.33.68.244/ROUTER/MESH
ROOT_PATH=`svn info --show-item repos-root-url`
echo "root path:[$ROOT_PATH]"

# 获取当前路径的url svn://10.33.68.244/ROUTER/MESH/branch/unify
CUR_PATH_URL=`svn info --show-item url`
echo "current path url:[$CUR_PATH_URL]"

# 获取前面的路径  /branch/unify
RELATIVE_URL=`svn info --show-item relative-url | cut -c2-`

# 获取svn log, 输出到文件
echo "get svn log..."
FILE=svnlog.txt
svn log -v -l10 --xml $CUR_PATH_URL > $FILE

# 运行svnlog程序
svnlog $ROOT_PATH `pwd`/$FILE
# 使用svn diff prod/cwmpd.c -c 2466 查看2466版本该文件的修改