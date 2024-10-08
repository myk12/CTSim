#!/bin/bash

dataPath="/tmp/scalability-test/"
# 创建一个输出文件，用于保存处理结果
output_file="output.csv"
echo "Node Count,Time(s),Memory(KB)" > "$output_file"

# 遍历/tmp/sac目录下的所有CSV文件
for file in $dataPath*.csv; do
    # 提取文件名中的数字部分作为结点个数
    node_count=$(basename "$file" | sed 's/[^0-9]//g')
    
    # 提取文件中的第二行，并将逗号替换为空格
    time_memory=$(sed -n '2p' "$file")
    
    # 将结点个数、Time(s)、Memory(KB)分别写入输出文件中的一行
    echo "$node_count,$time_memory" >> "$output_file"
done

# 对输出文件按照第一列的值进行排序（除了第一行）
sort -t, -k1n -o "$output_file" "$output_file"