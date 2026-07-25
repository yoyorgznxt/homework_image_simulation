#!/bin/bash

# GitHub 仓库地址（请修改为你的仓库）
GITHUB_REPO="git@github.com:yoyorgznxt/homework_image_simulation.git"

echo "=== 提交代码到 GitHub ==="

# 添加所有文件
git add .

# 提交
echo -n "请输入提交信息 (默认: update): "
read msg
if [ -z "$msg" ]; then
    msg="update"
fi
git commit -m "$msg"

# 推送
git push -u origin main

echo "=== 完成 ==="
