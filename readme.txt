1.在residenc,industry,commercial文件夹中加入了duration.txt加快读取速度

2.resiX_stepY中的结果表示读取residence文件夹中的第X个用例，最大准许延迟为Y小时

3.ga_log0.txt记录当前方案费用，ga_log1.txt记录最优方案费用

4.ga_bestgene.txt描述最优方案的基因，格式为：延后数量 延后时长

5.ga_use.txt记录最优方案下的用电量曲线

6.程序中涉及stage的部分是为了能够兼容阶梯电价模型，目前只设置了一阶，即无阶梯

7.最大准许延迟在程序开头的预编译部分修改MAX_DELAY

8.读取同区域的不同用例在程序main函数中修改initialize的参数，从1~10

9.读取不同区域的用例在程序开头的预编译部分修改RESIDENCE的注释