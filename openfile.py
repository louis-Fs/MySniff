import sys
import os
import re
txt_file_path = "http.txt"
write_txt_file_path = "result.txt"
def write_txt_file():
    if os.path.exists(txt_file_path) is False:
        return
    with open(txt_file_path,'r') as r_file:
        res_file = re.findall(r'username=(.*?)&',re.I)
        # for row in r_file:

        #     # list = row.split("\t")
        #     # # print("用\\t分割行的结果:{}".format(list))
        #     # # print("待分割的列:{}".format(list[2]))
        #     # col = list[2]
        #     # res = col[2:-2].split('|')
        #     # print("对此列进行分割的结果：{}".format(res))
        #     #将分割结果写入新的文件中
        #     with open(write_txt_file_path,'a') as w_file:
        #         w_file.write(' '.join(res))
        #         w_file.write('\n')
        #     #break
 
if __name__ == "__main__":
    write_txt_file()