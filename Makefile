#当前文件夹
CUR_DIR = .#$(shell pwd)
#源文件夹 可包含多个
SRC_DIR = $(CUR_DIR)/src
#头文件夹 可包含多个
INC_DIR = $(CUR_DIR)/inc
#可执行文件夹 
BIN_DIR = $(CUR_DIR)/bin
#中间文件夹 
OBJ_DIR = $(CUR_DIR)/obj
#依赖文件夹 
DEP_DIR = $(CUR_DIR)/obj
#可执行文件名
EXCU = manageStaff
#编译器: gcc g++
CC = gcc
#源文件后缀名: c cpp *
SUFFIX = c
#编译选项： 
#C常用标准： c89 c99 gnu90 gnu99 gnu11 
#C++常用标准： c++98 c++11 gnu++98 gnu++11
CFLAGS = -g -Wall -fexec-charset=utf-8 -std=gnu99 -lpthread
CPATH = $(addprefix -I,$(INC_DIR))
#--------------------------------------------------------------------
#文件集合
#INCS = $(wildcard $(addsuffix /*.h,$(INC_DIR)))
SRCS = $(wildcard $(addsuffix /*.$(SUFFIX),$(SRC_DIR)))
OBJS = $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(basename $(notdir $(SRCS)))))
DEPS = $(addprefix $(DEP_DIR)/,$(addsuffix .d,$(basename $(notdir $(SRCS)))))
BINS = $(BIN_DIR)/$(EXCU)
#Makefile路径设置
vpath
vpath %.c $(SRC_DIR)
vpath %.h $(INC_DIR)
vpath %.o $(OBJ_DIR)#重要 用于寻找目标%.o
vpath %.d $(DEP_DIR)#重要 用于寻找目标%.d

#第一个目标
all:$(BINS)

#包含依赖文件 没有则不包含 继续执行
-include $(DEPS)#文件名为%.d 内部目标名为 %.o 没有路径

#链接 生成可执行文件 利用 %.o 的依赖信息
$(BINS):$(notdir $(OBJS))
	$(CC) $(OBJS) $(CFLAGS) $(CPATH) -o $(BINS) 

#生成.o和.d文件
 %.o:%.c
	$(CC) $< $(CFLAGS) $(CPATH) -c -o $(OBJ_DIR)/$@ 
	@$(CC) $< $(CFLAGS) $(CPATH) -MM -o $(DEP_DIR)/$(basename $(@F)).d 

#清理文件
.PHONY:clean
clean:
	rm -f $(BINS) $(OBJS) $(DEPS)

#	$@	
# 表示规则的目标文件名。
# 如果目标是一个文档文件（Linux 中，一般成 .a 文件为文档文件，也成为静态的库文件），那么它代表这个文档的文件名。
# 在多目标模式规则中，它代表的是触发规则被执行的文件名。
#	$%	
# 当目标文件是一个静态库文件时，代表静态库的一个成员名。
#	$<	
# 规则的第一个依赖的文件名。如果是一个目标文件使用隐含的规则来重建，则它代表由隐含规则加入的第一个依赖文件。
#	$?	
# 所有比目标文件更新的依赖文件列表，空格分隔。如果目标文件时静态库文件，代表的是库文件（.o 文件）。
#	$^
# 代表的是所有依赖文件列表，使用空格分隔。如果目标是静态库文件，它所代表的只能是所有的库成员（.o 文件）名。
# 一个文件可重复的出现在目标的依赖中，变量“$^”只记录它的第一次引用的情况。就是说变量“$^”会去掉重复的依赖文件。
#	$+	
# 类似“$^”，但是它保留了依赖文件中重复出现的文件。主要用在程序链接时库的交叉引用场合。
#	$*
# 在模式规则和静态模式规则中，代表“茎”。“茎”是目标模式中“%”所代表的部分（当文件名中存在目录时，“茎”也包含目录部分）。
