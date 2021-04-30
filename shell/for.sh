#!/bin/sh

# var="bcd"
# if [ "abc" == $var ]
# then
#     echo equal $var
# elif [[ "bcd" == $var ]]
# then
#     echo equal $var
# else
#     echo unequal
# fi

# function def_func()
# {
#     local para=$1
#     echo "val:"$para
#     echo define_fun
#     return 2
# }
# var='aa1'
# case $var in
# "abc")
#     echo $var
# ;;
# 'bbb'|"asd")
#     echo $var
# ;;
# aa)
#     echo $var
# ;;
# *)
#     def_func "hello"
#     echo $?
# ;;
# esac


# for i in {1..10}
# do
#     echo $i
#     printf '%d\n' $i
# done

# for file in /lib/*;do echo $file;done

# for i in $(seq 5 -1 1)
# do
#     echo $i
# done

# i=0
# while ((i<10))
# do
#     echo $i
#     ((++i))
# done

# i=0
# while [ $i -lt 10 ]
# do
#     echo $i
#     let i++
# done

# i=0
# until [[ $i == 10 ]]
# do
#     echo $i
#     i=$[i+1]
# done

# echo $(ls)
# echo `ls`

# var='test'
# (var='notest';echo $var)
# echo $var

# var="test"
# { var="notest";echo ${var};}
# echo $var
